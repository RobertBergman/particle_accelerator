#pragma once

#include "physics/ParticleSystem.hpp"
#include "physics/Integrator.hpp"
#include "physics/EMField.hpp"
#include "accelerator/Accelerator.hpp"

#include <memory>
#include <functional>

namespace pas::physics {

/**
 * @brief Physics simulation state.
 */
enum class SimulationState {
    Stopped,
    Running,
    Paused
};

/**
 * @brief Statistics from the physics simulation.
 */
struct SimulationStats {
    double simulationTime = 0.0;        // Total simulated time [s]
    uint64_t stepCount = 0;             // Total integration steps
    double stepsPerSecond = 0.0;        // Performance metric
    size_t particleCount = 0;           // Current particle count
    size_t lostParticleCount = 0;       // Lost particles
    double averageEnergy = 0.0;         // Average particle energy [J]
    double energySpread = 0.0;          // Energy spread (RMS) [J]
};

/**
 * @brief Orchestrates the physics simulation.
 *
 * Manages particle systems, integrators, and time stepping.
 * Acts as the central controller for the physics simulation.
 */
class PhysicsEngine {
public:
    using LossCallback = std::function<void(const Particle&)>;

    PhysicsEngine();
    ~PhysicsEngine() = default;

    // Non-copyable
    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;

    /**
     * @brief Set the accelerator to simulate.
     */
    void setAccelerator(std::shared_ptr<accelerator::Accelerator> accelerator);

    /**
     * @brief Get the current accelerator.
     */
    std::shared_ptr<accelerator::Accelerator> getAccelerator() const { return m_accelerator; }

    /**
     * @brief Get the particle system.
     */
    ParticleSystem& getParticleSystem() { return m_particleSystem; }
    const ParticleSystem& getParticleSystem() const { return m_particleSystem; }

    /**
     * @brief Set the integration method.
     */
    void setIntegrator(IntegratorFactory::Type type);
    IntegratorFactory::Type getIntegratorType() const { return m_integratorType; }

    /**
     * @brief Set the time step for integration.
     */
    void setTimeStep(double dt) { m_timeStep = dt; }
    double getTimeStep() const { return m_timeStep; }

    /**
     * @brief Set the time scale multiplier.
     */
    void setTimeScale(double scale) { m_timeScale = std::max(0.0, scale); }
    double getTimeScale() const { return m_timeScale; }

    /**
     * @brief Set maximum integration steps per frame (prevents UI freeze).
     */
    void setMaxStepsPerFrame(size_t maxSteps) { m_maxStepsPerFrame = maxSteps; }
    size_t getMaxStepsPerFrame() const { return m_maxStepsPerFrame; }

    /**
     * @brief Control simulation state.
     */
    void start();
    void stop();
    void pause();
    void resume();
    void reset();

    SimulationState getState() const { return m_state; }
    bool isRunning() const { return m_state == SimulationState::Running; }
    bool isPaused() const { return m_state == SimulationState::Paused; }

    /**
     * @brief Advance simulation by one frame.
     * @param deltaTime Real-world time since last update [s]
     */
    void update(double deltaTime);

    /**
     * @brief Perform a single integration step.
     */
    void step();

    /**
     * @brief Get simulation statistics.
     */
    const SimulationStats& getStats() const { return m_stats; }

    /**
     * @brief Set callback for particle loss events.
     */
    void setLossCallback(LossCallback callback) { m_lossCallback = std::move(callback); }

    /**
     * @brief Initialize a default beam.
     */
    void initializeDefaultBeam();

private:
    void updateStats(double frameTime);
    void checkParticleLosses();

    ParticleSystem m_particleSystem;
    EMFieldManager m_fieldManager;
    std::shared_ptr<accelerator::Accelerator> m_accelerator;
    std::unique_ptr<Integrator> m_integrator;
    IntegratorFactory::Type m_integratorType = IntegratorFactory::Type::Boris;

    SimulationState m_state = SimulationState::Stopped;
    double m_timeStep = 1e-11;      // Default: 10 ps
    double m_timeScale = 1.0;       // Real-time multiplier
    double m_accumulatedTime = 0.0; // For fixed timestep
    double m_currentTime = 0.0;     // Simulation time
    size_t m_maxStepsPerFrame = 10000;  // Cap to keep UI responsive

    SimulationStats m_stats;
    LossCallback m_lossCallback;

    // Performance tracking
    double m_lastStepTime = 0.0;
    uint64_t m_stepsThisSecond = 0;
};

} // namespace pas::physics
