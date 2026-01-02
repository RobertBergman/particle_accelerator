#include "utils/Logger.hpp"
#include "utils/Timer.hpp"
#include "physics/Constants.hpp"

#include <iostream>

int main() {
    // Initialize logging
    pas::utils::Logger::init("PAS", pas::utils::Logger::Level::Debug);

    PAS_INFO("Particle Accelerator Simulation v1.0.0");
    PAS_INFO("=====================================");

    // Display some physical constants to verify setup
    using namespace pas::physics::constants;

    PAS_INFO("Physical Constants:");
    PAS_INFO("  Speed of light: {:.6e} m/s", c);
    PAS_INFO("  Elementary charge: {:.6e} C", e);
    PAS_INFO("  Electron mass: {:.6e} kg", m_e);
    PAS_INFO("  Proton mass: {:.6e} kg", m_p);
    PAS_INFO("  Electron rest energy: {:.3f} MeV", energy::E_e_MeV);
    PAS_INFO("  Proton rest energy: {:.3f} MeV", energy::E_p_MeV);

    // Test timer functionality
    PAS_INFO("Testing timer...");
    {
        pas::utils::ScopedTimer timer("Startup test");

        pas::utils::Timer t;
        // Small delay for testing
        volatile int sum = 0;
        for (int i = 0; i < 1000000; ++i) {
            sum += i;
        }
        (void)sum;

        PAS_DEBUG("Timer elapsed: {:.3f} ms", t.elapsedMilliseconds());
    }

    // Test relativistic calculations
    PAS_INFO("Testing relativistic calculations...");
    {
        // 7 TeV proton (LHC energy)
        double kineticEnergy = 7.0e12 * energy::eV; // 7 TeV in Joules
        double gamma = relativistic::gammaFromKineticEnergy(kineticEnergy, m_p);
        double beta = relativistic::betaFromGamma(gamma);
        double momentum = relativistic::momentumFromGamma(gamma, m_p);

        PAS_INFO("7 TeV Proton:");
        PAS_INFO("  Lorentz factor (gamma): {:.2f}", gamma);
        PAS_INFO("  Velocity (beta = v/c): {:.15f}", beta);
        PAS_INFO("  Momentum: {:.6e} kg*m/s", momentum);
    }

    PAS_INFO("=====================================");
    PAS_INFO("Phase 1 infrastructure complete!");
    PAS_INFO("Next: Implement Physics Engine (Phase 2)");

    // Shutdown logging
    pas::utils::Logger::shutdown();

    return 0;
}
