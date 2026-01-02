#pragma once

#include <cmath>
#include <numbers>

namespace pas::physics::constants {

// Pi constant (C++20)
constexpr double pi = std::numbers::pi;

/**
 * @brief Physical constants in SI units.
 *
 * All values are from CODATA 2018 recommended values.
 * See: https://physics.nist.gov/cuu/Constants/
 */

// Speed of light in vacuum (m/s)
constexpr double c = 299792458.0;

// Speed of light squared (m^2/s^2)
constexpr double c2 = c * c;

// Elementary charge (C)
constexpr double e = 1.602176634e-19;

// Electron mass (kg)
constexpr double m_e = 9.1093837015e-31;

// Proton mass (kg)
constexpr double m_p = 1.67262192369e-27;

// Neutron mass (kg)
constexpr double m_n = 1.67492749804e-27;

// Atomic mass unit (kg)
constexpr double u = 1.66053906660e-27;

// Vacuum permittivity (F/m)
constexpr double epsilon_0 = 8.8541878128e-12;

// Vacuum permeability (H/m)
constexpr double mu_0 = 1.25663706212e-6;

// Planck constant (J*s)
constexpr double h = 6.62607015e-34;

// Reduced Planck constant (J*s)
constexpr double hbar = h / (2.0 * pi);

// Boltzmann constant (J/K)
constexpr double k_B = 1.380649e-23;

// Avogadro constant (1/mol)
constexpr double N_A = 6.02214076e23;

// Fine structure constant (dimensionless)
constexpr double alpha = 7.2973525693e-3;

// Classical electron radius (m)
constexpr double r_e = 2.8179403262e-15;

// Bohr radius (m)
constexpr double a_0 = 5.29177210903e-11;

/**
 * @brief Energy conversion factors
 */
namespace energy {

// 1 electron volt in Joules
constexpr double eV = 1.602176634e-19;

// 1 keV in Joules
constexpr double keV = 1.602176634e-16;

// 1 MeV in Joules
constexpr double MeV = 1.602176634e-13;

// 1 GeV in Joules
constexpr double GeV = 1.602176634e-10;

// 1 TeV in Joules
constexpr double TeV = 1.602176634e-7;

// Electron rest energy (J)
constexpr double E_e = m_e * c2;

// Proton rest energy (J)
constexpr double E_p = m_p * c2;

// Electron rest energy (MeV)
constexpr double E_e_MeV = E_e / MeV;

// Proton rest energy (MeV)
constexpr double E_p_MeV = E_p / MeV;

/**
 * @brief Convert energy from eV to Joules.
 */
constexpr double eVtoJ(double energy_eV) {
    return energy_eV * eV;
}

/**
 * @brief Convert energy from Joules to eV.
 */
constexpr double JtoeV(double energy_J) {
    return energy_J / eV;
}

/**
 * @brief Convert energy from MeV to Joules.
 */
constexpr double MeVtoJ(double energy_MeV) {
    return energy_MeV * MeV;
}

/**
 * @brief Convert energy from Joules to MeV.
 */
constexpr double JtoMeV(double energy_J) {
    return energy_J / MeV;
}

} // namespace energy

/**
 * @brief Relativistic utility functions
 */
namespace relativistic {

/**
 * @brief Calculate Lorentz factor gamma from velocity.
 * @param v Velocity magnitude (m/s)
 * @return Lorentz factor gamma = 1/sqrt(1 - v^2/c^2)
 */
inline double gammaFromVelocity(double v) {
    double beta = v / c;
    return 1.0 / std::sqrt(1.0 - beta * beta);
}

/**
 * @brief Calculate Lorentz factor gamma from beta.
 * @param beta Velocity as fraction of c (v/c)
 * @return Lorentz factor gamma = 1/sqrt(1 - beta^2)
 */
inline double gammaFromBeta(double beta) {
    return 1.0 / std::sqrt(1.0 - beta * beta);
}

/**
 * @brief Calculate beta from Lorentz factor gamma.
 * @param gamma Lorentz factor
 * @return Beta = v/c = sqrt(1 - 1/gamma^2)
 */
inline double betaFromGamma(double gamma) {
    return std::sqrt(1.0 - 1.0 / (gamma * gamma));
}

/**
 * @brief Calculate Lorentz factor gamma from kinetic energy and rest mass.
 * @param kineticEnergy Kinetic energy (J)
 * @param restMass Rest mass (kg)
 * @return Lorentz factor gamma = 1 + Ek/(m*c^2)
 */
inline double gammaFromKineticEnergy(double kineticEnergy, double restMass) {
    return 1.0 + kineticEnergy / (restMass * c2);
}

/**
 * @brief Calculate kinetic energy from Lorentz factor gamma and rest mass.
 * @param gamma Lorentz factor
 * @param restMass Rest mass (kg)
 * @return Kinetic energy (J) = (gamma - 1) * m * c^2
 */
inline double kineticEnergyFromGamma(double gamma, double restMass) {
    return (gamma - 1.0) * restMass * c2;
}

/**
 * @brief Calculate total energy from Lorentz factor gamma and rest mass.
 * @param gamma Lorentz factor
 * @param restMass Rest mass (kg)
 * @return Total energy (J) = gamma * m * c^2
 */
inline double totalEnergyFromGamma(double gamma, double restMass) {
    return gamma * restMass * c2;
}

/**
 * @brief Calculate momentum magnitude from Lorentz factor and rest mass.
 * @param gamma Lorentz factor
 * @param restMass Rest mass (kg)
 * @return Momentum magnitude (kg*m/s) = gamma * m * v = gamma * beta * m * c
 */
inline double momentumFromGamma(double gamma, double restMass) {
    double beta = betaFromGamma(gamma);
    return gamma * beta * restMass * c;
}

/**
 * @brief Calculate Lorentz factor from momentum and rest mass.
 * @param momentum Momentum magnitude (kg*m/s)
 * @param restMass Rest mass (kg)
 * @return Lorentz factor gamma = sqrt(1 + (p/(m*c))^2)
 */
inline double gammaFromMomentum(double momentum, double restMass) {
    double pOverMc = momentum / (restMass * c);
    return std::sqrt(1.0 + pOverMc * pOverMc);
}

} // namespace relativistic

} // namespace pas::physics::constants
