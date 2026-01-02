#include <gtest/gtest.h>
#include <cmath>

#include "physics/ParticleSystem.hpp"
#include "physics/Constants.hpp"

namespace pas::physics::tests {

using namespace constants;

class ParticleSystemTest : public ::testing::Test {
protected:
    ParticleSystem system;

    BeamParameters createDefaultParams() {
        BeamParameters params;
        params.particleType = BeamParameters::ParticleType::Proton;
        params.numParticles = 100;
        params.kineticEnergy = 1.0 * energy::GeV;
        params.sigmaX = 1e-3;
        params.sigmaY = 1e-3;
        params.sigmaZ = 1e-2;
        params.seed = 42;
        return params;
    }
};

// Basic operations

TEST_F(ParticleSystemTest, InitiallyEmpty) {
    EXPECT_EQ(system.getParticleCount(), 0u);
    EXPECT_EQ(system.getActiveParticleCount(), 0u);
}

TEST_F(ParticleSystemTest, AddParticle) {
    Particle p = Particle::proton();
    system.addParticle(p);

    EXPECT_EQ(system.getParticleCount(), 1u);
}

TEST_F(ParticleSystemTest, ClearRemovesParticles) {
    system.addParticle(Particle::proton());
    system.addParticle(Particle::electron());
    EXPECT_EQ(system.getParticleCount(), 2u);

    system.clear();
    EXPECT_EQ(system.getParticleCount(), 0u);
}

// Beam generation

TEST_F(ParticleSystemTest, GenerateBeamCreatesCorrectCount) {
    BeamParameters params = createDefaultParams();
    params.numParticles = 500;

    system.generateBeam(params);

    EXPECT_EQ(system.getParticleCount(), 500u);
    EXPECT_EQ(system.getActiveParticleCount(), 500u);
}

TEST_F(ParticleSystemTest, GenerateProtonBeam) {
    BeamParameters params = createDefaultParams();
    params.particleType = BeamParameters::ParticleType::Proton;
    params.numParticles = 10;

    system.generateBeam(params);

    for (const auto& p : system.getParticles()) {
        EXPECT_DOUBLE_EQ(p.getMass(), m_p);
        EXPECT_DOUBLE_EQ(p.getCharge(), e);
    }
}

TEST_F(ParticleSystemTest, GenerateElectronBeam) {
    BeamParameters params = createDefaultParams();
    params.particleType = BeamParameters::ParticleType::Electron;
    params.numParticles = 10;

    system.generateBeam(params);

    for (const auto& p : system.getParticles()) {
        EXPECT_DOUBLE_EQ(p.getMass(), m_e);
        EXPECT_DOUBLE_EQ(p.getCharge(), -e);
    }
}

TEST_F(ParticleSystemTest, BeamIsReproducible) {
    BeamParameters params = createDefaultParams();
    params.numParticles = 50;
    params.seed = 12345;

    system.generateBeam(params);
    std::vector<glm::dvec3> positions1;
    for (const auto& p : system.getParticles()) {
        positions1.push_back(p.getPosition());
    }

    // Generate again with same seed
    system.generateBeam(params);
    std::vector<glm::dvec3> positions2;
    for (const auto& p : system.getParticles()) {
        positions2.push_back(p.getPosition());
    }

    EXPECT_EQ(positions1.size(), positions2.size());
    for (size_t i = 0; i < positions1.size(); ++i) {
        EXPECT_DOUBLE_EQ(positions1[i].x, positions2[i].x);
        EXPECT_DOUBLE_EQ(positions1[i].y, positions2[i].y);
        EXPECT_DOUBLE_EQ(positions1[i].z, positions2[i].z);
    }
}

TEST_F(ParticleSystemTest, DifferentSeedsGiveDifferentBeams) {
    BeamParameters params = createDefaultParams();
    params.numParticles = 10;

    params.seed = 1;
    system.generateBeam(params);
    glm::dvec3 pos1 = system.getParticle(0).getPosition();

    params.seed = 2;
    system.generateBeam(params);
    glm::dvec3 pos2 = system.getParticle(0).getPosition();

    // Positions should differ
    bool different = (pos1.x != pos2.x) || (pos1.y != pos2.y) || (pos1.z != pos2.z);
    EXPECT_TRUE(different);
}

TEST_F(ParticleSystemTest, BeamHasCorrectMeanEnergy) {
    BeamParameters params = createDefaultParams();
    params.numParticles = 1000;
    params.kineticEnergy = 100.0 * energy::MeV;
    params.sigmaDelta = 0.0;  // No energy spread

    system.generateBeam(params);

    BeamStatistics stats = system.computeStatistics();

    // Mean energy should be close to target
    double relativeError = std::abs(stats.meanEnergy - params.kineticEnergy) / params.kineticEnergy;
    EXPECT_LT(relativeError, 0.01);  // Within 1%
}

// Aperture

TEST_F(ParticleSystemTest, ApertureLosesOutsideParticles) {
    system.addParticle(Particle::proton(glm::dvec3(0.0, 0.0, 0.0)));
    system.addParticle(Particle::proton(glm::dvec3(0.05, 0.0, 0.0)));
    system.addParticle(Particle::proton(glm::dvec3(0.2, 0.0, 0.0)));

    size_t lost = system.applyAperture(0.1);

    EXPECT_EQ(lost, 1u);
    EXPECT_EQ(system.getActiveParticleCount(), 2u);
}

TEST_F(ParticleSystemTest, IsWithinAperture) {
    Particle inside = Particle::proton(glm::dvec3(0.05, 0.05, 0.0));
    Particle outside = Particle::proton(glm::dvec3(0.15, 0.15, 0.0));

    EXPECT_TRUE(ParticleSystem::isWithinAperture(inside, 0.1));
    EXPECT_FALSE(ParticleSystem::isWithinAperture(outside, 0.1));
}

// Inactive particles

TEST_F(ParticleSystemTest, CountsActiveParticles) {
    system.addParticle(Particle::proton());
    system.addParticle(Particle::proton());

    Particle inactive = Particle::proton();
    inactive.setActive(false);
    system.addParticle(inactive);

    EXPECT_EQ(system.getParticleCount(), 3u);
    EXPECT_EQ(system.getActiveParticleCount(), 2u);
}

TEST_F(ParticleSystemTest, RemoveInactiveParticles) {
    system.addParticle(Particle::proton());

    Particle inactive = Particle::proton();
    inactive.setActive(false);
    system.addParticle(inactive);

    system.removeInactiveParticles();

    EXPECT_EQ(system.getParticleCount(), 1u);
}

// Statistics

TEST_F(ParticleSystemTest, StatisticsOnEmptySystem) {
    BeamStatistics stats = system.computeStatistics();

    EXPECT_EQ(stats.totalParticles, 0u);
    EXPECT_EQ(stats.activeParticles, 0u);
}

TEST_F(ParticleSystemTest, StatisticsCountsCorrectly) {
    BeamParameters params = createDefaultParams();
    params.numParticles = 100;
    system.generateBeam(params);

    // Deactivate some
    for (size_t i = 0; i < 10; ++i) {
        system.getParticle(i).setActive(false);
    }

    BeamStatistics stats = system.computeStatistics();

    EXPECT_EQ(stats.totalParticles, 100u);
    EXPECT_EQ(stats.activeParticles, 90u);
    EXPECT_EQ(stats.lostParticles, 10u);
}

TEST_F(ParticleSystemTest, MeanPositionIsComputed) {
    Particle p1 = Particle::proton(glm::dvec3(1.0, 0.0, 0.0));
    Particle p2 = Particle::proton(glm::dvec3(3.0, 0.0, 0.0));

    system.addParticle(p1);
    system.addParticle(p2);

    BeamStatistics stats = system.computeStatistics();

    EXPECT_DOUBLE_EQ(stats.meanPosition.x, 2.0);
}

TEST_F(ParticleSystemTest, RMSSizeIsComputed) {
    // Create particles at fixed positions
    system.addParticle(Particle::proton(glm::dvec3(-1.0, 0.0, 0.0)));
    system.addParticle(Particle::proton(glm::dvec3(1.0, 0.0, 0.0)));

    BeamStatistics stats = system.computeStatistics();

    // RMS should be 1.0 for this symmetric case
    EXPECT_NEAR(stats.rmsSize.x, 1.0, 1e-10);
}

// Reference momentum

TEST_F(ParticleSystemTest, ReferenceMomentumSetAfterGeneration) {
    BeamParameters params = createDefaultParams();
    params.kineticEnergy = 1.0 * energy::GeV;

    system.generateBeam(params);

    double refP = system.getReferenceMomentum();
    EXPECT_GT(refP, 0.0);

    // Should match momentum of a particle with given energy
    double gamma = relativistic::gammaFromKineticEnergy(params.kineticEnergy, m_p);
    double beta = relativistic::betaFromGamma(gamma);
    double expectedP = gamma * beta * m_p * c;

    EXPECT_NEAR(refP, expectedP, expectedP * 1e-10);
}

// Distribution types

TEST_F(ParticleSystemTest, UniformDistribution) {
    BeamParameters params = createDefaultParams();
    params.distribution = BeamParameters::Distribution::Uniform;
    params.numParticles = 1000;

    system.generateBeam(params);

    BeamStatistics stats = system.computeStatistics();

    // Mean should be close to 0
    EXPECT_NEAR(stats.meanPosition.x, 0.0, params.sigmaX);
    EXPECT_NEAR(stats.meanPosition.y, 0.0, params.sigmaY);
}

TEST_F(ParticleSystemTest, GaussianDistribution) {
    BeamParameters params = createDefaultParams();
    params.distribution = BeamParameters::Distribution::Gaussian;
    params.numParticles = 10000;

    system.generateBeam(params);

    BeamStatistics stats = system.computeStatistics();

    // RMS should be close to sigma for Gaussian
    double tolerance = 0.1;  // 10% tolerance for statistical fluctuations
    EXPECT_NEAR(stats.rmsSize.x, params.sigmaX, params.sigmaX * tolerance);
    EXPECT_NEAR(stats.rmsSize.y, params.sigmaY, params.sigmaY * tolerance);
}

} // namespace pas::physics::tests
