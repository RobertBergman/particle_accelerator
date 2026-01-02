#include "physics/PhysicsEngine.hpp"
#include "utils/Logger.hpp"

#include <cmath>
#include <algorithm>

namespace pas::physics {

PhysicsEngine::PhysicsEngine() {
    // Initialize default integrator
    setIntegrator(IntegratorFactory::Type::Boris);
}

void PhysicsEngine::setAccelerator(std::shared_ptr<accelerator::Accelerator> accelerator) {
    m_accelerator = std::move(accelerator);

    // Update field manager with accelerator's fields
    if (m_accelerator) {
        m_fieldManager.clear();
        m_accelerator->populateFieldManager(m_fieldManager);
        PAS_DEBUG("PhysicsEngine: Set accelerator with {} components", m_accelerator->getComponentCount());
    }
}

void PhysicsEngine::setIntegrator(IntegratorFactory::Type type) {
    m_integratorType = type;
    m_integrator = IntegratorFactory::create(type);
    PAS_DEBUG("PhysicsEngine: Set integrator to {}", static_cast<int>(type));
}

void PhysicsEngine::start() {
    if (m_state == SimulationState::Stopped) {
        reset();
    }
    m_state = SimulationState::Running;
    PAS_INFO("PhysicsEngine: Simulation started");
}

void PhysicsEngine::stop() {
    m_state = SimulationState::Stopped;
    PAS_INFO("PhysicsEngine: Simulation stopped (time: {:.6e} s, steps: {})",
             m_stats.simulationTime, m_stats.stepCount);
}

void PhysicsEngine::pause() {
    if (m_state == SimulationState::Running) {
        m_state = SimulationState::Paused;
        PAS_INFO("PhysicsEngine: Simulation paused");
    }
}

void PhysicsEngine::resume() {
    if (m_state == SimulationState::Paused) {
        m_state = SimulationState::Running;
        PAS_INFO("PhysicsEngine: Simulation resumed");
    }
}

void PhysicsEngine::reset() {
    m_stats = SimulationStats{};
    m_accumulatedTime = 0.0;
    m_currentTime = 0.0;
    m_stepsThisSecond = 0;
    m_lastStepTime = 0.0;

    // Clear particles
    m_particleSystem.clear();

    PAS_INFO("PhysicsEngine: Simulation reset");
}

void PhysicsEngine::update(double deltaTime) {
    if (m_state != SimulationState::Running) {
        return;
    }

    // Scale time
    double scaledDelta = deltaTime * m_timeScale;

    // Accumulate time for fixed timestep integration
    m_accumulatedTime += scaledDelta;

    // Perform fixed timesteps (capped to prevent UI freeze)
    size_t stepsThisFrame = 0;
    while (m_accumulatedTime >= m_timeStep && stepsThisFrame < m_maxStepsPerFrame) {
        step();
        m_accumulatedTime -= m_timeStep;
        stepsThisFrame++;
    }

    // If we hit the cap, discard excess accumulated time to prevent runaway
    if (stepsThisFrame >= m_maxStepsPerFrame && m_accumulatedTime > m_timeStep) {
        m_accumulatedTime = 0.0;
    }

    // Update performance stats
    updateStats(deltaTime);
}

void PhysicsEngine::step() {
    if (!m_integrator) {
        return;
    }

    auto& particles = m_particleSystem.getParticles();

    // Integrate each particle
    for (auto& particle : particles) {
        if (!particle.isActive()) {
            continue;
        }

        // Integrate using the integrator
        m_integrator->step(particle, m_fieldManager, m_currentTime, m_timeStep);
    }

    // Check for particle losses
    checkParticleLosses();

    // Update stats
    m_currentTime += m_timeStep;
    m_stats.simulationTime = m_currentTime;
    m_stats.stepCount++;
    m_stepsThisSecond++;
}

void PhysicsEngine::updateStats(double frameTime) {
    m_lastStepTime += frameTime;

    // Update steps per second every second
    if (m_lastStepTime >= 1.0) {
        m_stats.stepsPerSecond = static_cast<double>(m_stepsThisSecond) / m_lastStepTime;
        m_stepsThisSecond = 0;
        m_lastStepTime = 0.0;
    }

    // Update particle statistics
    m_stats.particleCount = m_particleSystem.getActiveParticleCount();

    auto beamStats = m_particleSystem.computeStatistics();
    m_stats.averageEnergy = beamStats.meanEnergy;
    m_stats.energySpread = beamStats.rmsEnergy;
}

void PhysicsEngine::checkParticleLosses() {
    if (!m_accelerator) {
        return;
    }

    auto& particles = m_particleSystem.getParticles();
    const auto& components = m_accelerator->getComponents();

    for (auto& particle : particles) {
        if (!particle.isActive()) {
            continue;
        }

        glm::dvec3 pos = particle.getPosition();

        // Check if particle is inside any component's aperture
        bool insideAperture = false;
        for (const auto& comp : components) {
            if (comp->isInsideAperture(pos)) {
                insideAperture = true;
                break;
            }
        }

        // If not in any aperture and we have components, particle may be lost
        if (!insideAperture && !components.empty()) {
            // Check distance from beam axis as fallback
            double radialDist = std::sqrt(pos.x * pos.x + pos.y * pos.y);
            if (radialDist > 0.1) {  // 10 cm default aperture
                particle.setActive(false);
                m_stats.lostParticleCount++;

                if (m_lossCallback) {
                    m_lossCallback(particle);
                }
            }
        }
    }
}

void PhysicsEngine::initializeDefaultBeam() {
    // Create a proton beam
    BeamParameters params;
    params.particleType = BeamParameters::ParticleType::Proton;
    params.kineticEnergy = 1.0e9 * constants::energy::eV;  // 1 GeV
    params.numParticles = 1000;
    params.sigmaX = 0.001;      // 1 mm
    params.sigmaY = 0.001;      // 1 mm
    params.sigmaZ = 0.01;       // 1 cm bunch length
    params.sigmaPx = 1e-4;      // 0.01% angular spread
    params.sigmaPy = 1e-4;      // 0.01% angular spread
    params.sigmaDelta = 0.001;  // 0.1% dp/p

    m_particleSystem.generateBeam(params);

    PAS_INFO("PhysicsEngine: Initialized default beam with {} particles at {:.3f} GeV",
             params.numParticles, params.kineticEnergy / (1e9 * constants::energy::eV));
}

} // namespace pas::physics
