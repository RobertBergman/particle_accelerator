#include <gtest/gtest.h>
#include <cmath>

#include "physics/Constants.hpp"

namespace pas::physics::tests {

using namespace pas::physics::constants;

class ConstantsTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-10;
    static constexpr double REL_EPSILON = 1e-6;
};

// Fundamental constants tests

TEST_F(ConstantsTest, SpeedOfLightIsCorrect) {
    EXPECT_DOUBLE_EQ(c, 299792458.0);
}

TEST_F(ConstantsTest, ElementaryChargeIsCorrect) {
    EXPECT_NEAR(e, 1.602176634e-19, 1e-28);
}

TEST_F(ConstantsTest, ElectronMassIsCorrect) {
    EXPECT_NEAR(m_e, 9.1093837015e-31, 1e-40);
}

TEST_F(ConstantsTest, ProtonMassIsCorrect) {
    EXPECT_NEAR(m_p, 1.67262192369e-27, 1e-37);
}

TEST_F(ConstantsTest, SpeedOfLightSquaredIsConsistent) {
    EXPECT_DOUBLE_EQ(c2, c * c);
}

// Energy conversion tests

TEST_F(ConstantsTest, eVConversionIsCorrect) {
    EXPECT_NEAR(energy::eV, 1.602176634e-19, 1e-28);
}

TEST_F(ConstantsTest, MeVConversionIsCorrect) {
    EXPECT_NEAR(energy::MeV, energy::eV * 1e6, 1e-22);
}

TEST_F(ConstantsTest, GeVConversionIsCorrect) {
    EXPECT_NEAR(energy::GeV, energy::eV * 1e9, 1e-19);
}

TEST_F(ConstantsTest, TeVConversionIsCorrect) {
    EXPECT_NEAR(energy::TeV, energy::eV * 1e12, 1e-16);
}

TEST_F(ConstantsTest, ElectronRestEnergyIsCorrect) {
    // Electron rest energy should be approximately 0.511 MeV
    EXPECT_NEAR(energy::E_e_MeV, 0.511, 0.001);
}

TEST_F(ConstantsTest, ProtonRestEnergyIsCorrect) {
    // Proton rest energy should be approximately 938.3 MeV
    EXPECT_NEAR(energy::E_p_MeV, 938.3, 0.1);
}

TEST_F(ConstantsTest, eVtoJRoundTrip) {
    double original = 1000.0; // 1 keV
    double joules = energy::eVtoJ(original);
    double back = energy::JtoeV(joules);
    EXPECT_NEAR(back, original, EPSILON);
}

TEST_F(ConstantsTest, MeVtoJRoundTrip) {
    double original = 100.0; // 100 MeV
    double joules = energy::MeVtoJ(original);
    double back = energy::JtoMeV(joules);
    EXPECT_NEAR(back, original, EPSILON);
}

// Relativistic calculations tests

TEST_F(ConstantsTest, GammaFromBetaAtRest) {
    // At rest (beta = 0), gamma should be 1
    double gamma = relativistic::gammaFromBeta(0.0);
    EXPECT_DOUBLE_EQ(gamma, 1.0);
}

TEST_F(ConstantsTest, GammaFromBetaAtLowVelocity) {
    // At low velocity, gamma should be close to 1
    double beta = 0.01; // 1% speed of light
    double gamma = relativistic::gammaFromBeta(beta);
    EXPECT_NEAR(gamma, 1.0, 0.001);
    EXPECT_GT(gamma, 1.0);
}

TEST_F(ConstantsTest, GammaFromBetaAtHighVelocity) {
    // At 0.99c, gamma should be approximately 7.09
    double beta = 0.99;
    double gamma = relativistic::gammaFromBeta(beta);
    EXPECT_NEAR(gamma, 7.09, 0.1);
}

TEST_F(ConstantsTest, BetaFromGammaInverse) {
    // Test that betaFromGamma is inverse of gammaFromBeta
    double originalBeta = 0.8;
    double gamma = relativistic::gammaFromBeta(originalBeta);
    double recoveredBeta = relativistic::betaFromGamma(gamma);
    EXPECT_NEAR(recoveredBeta, originalBeta, EPSILON);
}

TEST_F(ConstantsTest, GammaFromVelocityConsistent) {
    double velocity = 0.5 * c;
    double gammaFromV = relativistic::gammaFromVelocity(velocity);
    double gammaFromBeta = relativistic::gammaFromBeta(0.5);
    EXPECT_NEAR(gammaFromV, gammaFromBeta, EPSILON);
}

TEST_F(ConstantsTest, KineticEnergyAtRest) {
    // Kinetic energy at gamma=1 should be 0
    double ke = relativistic::kineticEnergyFromGamma(1.0, m_e);
    EXPECT_DOUBLE_EQ(ke, 0.0);
}

TEST_F(ConstantsTest, KineticEnergyRoundTrip) {
    double originalKE = 10.0 * energy::MeV;
    double gamma = relativistic::gammaFromKineticEnergy(originalKE, m_p);
    double recoveredKE = relativistic::kineticEnergyFromGamma(gamma, m_p);
    EXPECT_NEAR(recoveredKE, originalKE, 1e-20);
}

TEST_F(ConstantsTest, TotalEnergyConsistent) {
    // Total energy = kinetic energy + rest energy
    double gamma = 2.0;
    double totalE = relativistic::totalEnergyFromGamma(gamma, m_e);
    double kineticE = relativistic::kineticEnergyFromGamma(gamma, m_e);
    double restE = m_e * c2;

    EXPECT_NEAR(totalE, kineticE + restE, 1e-25);
}

TEST_F(ConstantsTest, MomentumAtRest) {
    // At rest (gamma=1), momentum should be 0
    double p = relativistic::momentumFromGamma(1.0, m_e);
    EXPECT_DOUBLE_EQ(p, 0.0);
}

TEST_F(ConstantsTest, MomentumRoundTrip) {
    double originalGamma = 5.0;
    double momentum = relativistic::momentumFromGamma(originalGamma, m_p);
    double recoveredGamma = relativistic::gammaFromMomentum(momentum, m_p);
    EXPECT_NEAR(recoveredGamma, originalGamma, EPSILON);
}

// Real-world physics validation tests

TEST_F(ConstantsTest, LHCProtonGamma) {
    // LHC operates at 7 TeV per beam
    // For a proton, this gives gamma ≈ 7461
    double kineticEnergy = 7.0e12 * energy::eV; // 7 TeV
    double gamma = relativistic::gammaFromKineticEnergy(kineticEnergy, m_p);

    EXPECT_NEAR(gamma, 7461, 10);
}

TEST_F(ConstantsTest, LHCProtonBeta) {
    // At LHC energies, protons travel very close to c
    double kineticEnergy = 7.0e12 * energy::eV;
    double gamma = relativistic::gammaFromKineticEnergy(kineticEnergy, m_p);
    double beta = relativistic::betaFromGamma(gamma);

    // Should be 0.999999...
    EXPECT_GT(beta, 0.999999);
    EXPECT_LT(beta, 1.0);
}

TEST_F(ConstantsTest, ElectronClassicalLimitValid) {
    // At non-relativistic speeds, classical approximations should work
    double velocity = 0.01 * c; // 1% of c
    double gamma = relativistic::gammaFromVelocity(velocity);

    // Classical kinetic energy: 0.5 * m * v^2
    double classicalKE = 0.5 * m_e * velocity * velocity;

    // Relativistic kinetic energy
    double relativisticKE = relativistic::kineticEnergyFromGamma(gamma, m_e);

    // At low velocities, these should be very close
    double relativeError = std::abs(relativisticKE - classicalKE) / classicalKE;
    EXPECT_LT(relativeError, 0.0001); // Less than 0.01% error
}

TEST_F(ConstantsTest, EnergyMomentumRelation) {
    // E^2 = (pc)^2 + (mc^2)^2
    double gamma = 3.0;
    double totalE = relativistic::totalEnergyFromGamma(gamma, m_e);
    double momentum = relativistic::momentumFromGamma(gamma, m_e);
    double restE = m_e * c2;

    double E2 = totalE * totalE;
    double pc2 = (momentum * c) * (momentum * c);
    double mc4 = restE * restE;

    EXPECT_NEAR(E2, pc2 + mc4, 1e-40);
}

} // namespace pas::physics::tests
