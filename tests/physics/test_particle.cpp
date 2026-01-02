#include <gtest/gtest.h>
#include <cmath>

#include "physics/Particle.hpp"
#include "physics/Constants.hpp"

namespace pas::physics::tests {

using namespace constants;

class ParticleTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-10;
};

// Construction tests

TEST_F(ParticleTest, ElectronFactoryCreatesCorrectParticle) {
    Particle p = Particle::electron();

    EXPECT_DOUBLE_EQ(p.getMass(), m_e);
    EXPECT_DOUBLE_EQ(p.getCharge(), -e);
    EXPECT_TRUE(p.isActive());
}

TEST_F(ParticleTest, ProtonFactoryCreatesCorrectParticle) {
    Particle p = Particle::proton();

    EXPECT_DOUBLE_EQ(p.getMass(), m_p);
    EXPECT_DOUBLE_EQ(p.getCharge(), e);
}

TEST_F(ParticleTest, PositronHasPositiveCharge) {
    Particle p = Particle::positron();

    EXPECT_DOUBLE_EQ(p.getMass(), m_e);
    EXPECT_DOUBLE_EQ(p.getCharge(), e);
}

TEST_F(ParticleTest, AntiprotonHasNegativeCharge) {
    Particle p = Particle::antiproton();

    EXPECT_DOUBLE_EQ(p.getMass(), m_p);
    EXPECT_DOUBLE_EQ(p.getCharge(), -e);
}

TEST_F(ParticleTest, InitialPositionIsSet) {
    glm::dvec3 pos(1.0, 2.0, 3.0);
    Particle p = Particle::proton(pos);

    EXPECT_DOUBLE_EQ(p.getX(), 1.0);
    EXPECT_DOUBLE_EQ(p.getY(), 2.0);
    EXPECT_DOUBLE_EQ(p.getZ(), 3.0);
}

TEST_F(ParticleTest, InitialMomentumIsSet) {
    glm::dvec3 pos(0.0);
    glm::dvec3 mom(1e-20, 2e-20, 3e-20);
    Particle p = Particle::proton(pos, mom);

    EXPECT_DOUBLE_EQ(p.getPx(), 1e-20);
    EXPECT_DOUBLE_EQ(p.getPy(), 2e-20);
    EXPECT_DOUBLE_EQ(p.getPz(), 3e-20);
}

// Relativistic calculations

TEST_F(ParticleTest, ParticleAtRestHasGammaOne) {
    Particle p = Particle::electron();

    EXPECT_DOUBLE_EQ(p.getGamma(), 1.0);
    EXPECT_DOUBLE_EQ(p.getBeta(), 0.0);
}

TEST_F(ParticleTest, ParticleAtRestHasZeroKineticEnergy) {
    Particle p = Particle::proton();

    EXPECT_DOUBLE_EQ(p.getKineticEnergy(), 0.0);
}

TEST_F(ParticleTest, RestEnergyIsCorrect) {
    Particle electron = Particle::electron();
    Particle proton = Particle::proton();

    EXPECT_NEAR(electron.getRestEnergy(), energy::E_e, 1e-30);
    EXPECT_NEAR(proton.getRestEnergy(), energy::E_p, 1e-20);
}

TEST_F(ParticleTest, SetKineticEnergyUpdatesGamma) {
    Particle p = Particle::proton();
    double ke = 1.0 * energy::GeV;  // 1 GeV

    p.setKineticEnergy(ke);

    // gamma = 1 + Ek / (m*c^2)
    double expectedGamma = 1.0 + ke / (m_p * c2);
    EXPECT_NEAR(p.getGamma(), expectedGamma, 1e-6);
}

TEST_F(ParticleTest, SetKineticEnergyRoundTrip) {
    Particle p = Particle::proton();
    double ke = 7.0 * energy::TeV;  // 7 TeV (LHC energy)

    p.setKineticEnergy(ke);
    double recovered = p.getKineticEnergy();

    EXPECT_NEAR(recovered, ke, ke * 1e-10);
}

TEST_F(ParticleTest, MomentumVelocityConsistency) {
    Particle p = Particle::electron();
    glm::dvec3 vel(0.5 * c, 0.0, 0.0);

    p.setVelocity(vel);

    // p = gamma * m * v
    double gamma = p.getGamma();
    double expectedP = gamma * m_e * 0.5 * c;

    EXPECT_NEAR(p.getMomentumMagnitude(), expectedP, expectedP * 1e-10);
}

TEST_F(ParticleTest, VelocityClampedBelowSpeedOfLight) {
    Particle p = Particle::proton();

    // Try to set velocity >= c
    p.setVelocity(glm::dvec3(c, 0.0, 0.0));

    EXPECT_LT(p.getBeta(), 1.0);
    EXPECT_LT(p.getSpeed(), c);
}

TEST_F(ParticleTest, TotalEnergyEqualsRestPlusKinetic) {
    Particle p = Particle::proton();
    p.setKineticEnergy(500.0 * energy::MeV);

    double totalE = p.getTotalEnergy();
    double kineticE = p.getKineticEnergy();
    double restE = p.getRestEnergy();

    EXPECT_NEAR(totalE, kineticE + restE, 1e-20);
}

// Delta calculation

TEST_F(ParticleTest, DeltaIsZeroAtReferenceMomentum) {
    Particle p = Particle::proton();
    double refP = 1e-18;  // Some reference momentum
    p.setMomentum(glm::dvec3(0.0, 0.0, refP));

    EXPECT_NEAR(p.getDelta(refP), 0.0, 1e-10);
}

TEST_F(ParticleTest, DeltaIsPositiveAboveReference) {
    Particle p = Particle::proton();
    double refP = 1e-18;
    p.setMomentum(glm::dvec3(0.0, 0.0, refP * 1.1));  // 10% above

    EXPECT_NEAR(p.getDelta(refP), 0.1, 1e-10);
}

TEST_F(ParticleTest, DeltaIsNegativeBelowReference) {
    Particle p = Particle::proton();
    double refP = 1e-18;
    p.setMomentum(glm::dvec3(0.0, 0.0, refP * 0.9));  // 10% below

    EXPECT_NEAR(p.getDelta(refP), -0.1, 1e-10);
}

// State management

TEST_F(ParticleTest, ParticleCanBeDeactivated) {
    Particle p = Particle::electron();
    EXPECT_TRUE(p.isActive());

    p.setActive(false);
    EXPECT_FALSE(p.isActive());
}

TEST_F(ParticleTest, ParticlesHaveUniqueIds) {
    Particle p1 = Particle::electron();
    Particle p2 = Particle::electron();

    EXPECT_NE(p1.getId(), p2.getId());
}

// Physics validation

TEST_F(ParticleTest, LHCProtonGammaMatches) {
    Particle p = Particle::proton();
    p.setKineticEnergy(7.0 * energy::TeV);

    // LHC protons at 7 TeV have gamma ≈ 7461
    EXPECT_NEAR(p.getGamma(), 7461.0, 10.0);
}

TEST_F(ParticleTest, LHCProtonBetaIsNearlyOne) {
    Particle p = Particle::proton();
    p.setKineticEnergy(7.0 * energy::TeV);

    // Should be very close to 1
    EXPECT_GT(p.getBeta(), 0.999999);
    EXPECT_LT(p.getBeta(), 1.0);
}

} // namespace pas::physics::tests
