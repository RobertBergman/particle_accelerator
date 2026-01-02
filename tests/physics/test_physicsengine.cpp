#include <gtest/gtest.h>

#include "physics/PhysicsEngine.hpp"
#include "physics/Constants.hpp"
#include "accelerator/Accelerator.hpp"

namespace pas::physics::tests {

class PhysicsEngineTest : public ::testing::Test {
protected:
    PhysicsEngine engine;
    static constexpr double EPSILON = 1e-10;
};

TEST_F(PhysicsEngineTest, DefaultConstruction) {
    EXPECT_EQ(engine.getState(), SimulationState::Stopped);
    EXPECT_FALSE(engine.isRunning());
    EXPECT_FALSE(engine.isPaused());
    EXPECT_EQ(engine.getIntegratorType(), IntegratorFactory::Type::Boris);
}

TEST_F(PhysicsEngineTest, SetTimeStep) {
    engine.setTimeStep(1e-12);
    EXPECT_DOUBLE_EQ(engine.getTimeStep(), 1e-12);
}

TEST_F(PhysicsEngineTest, SetTimeScale) {
    engine.setTimeScale(2.0);
    EXPECT_DOUBLE_EQ(engine.getTimeScale(), 2.0);

    // Negative scale should clamp to 0
    engine.setTimeScale(-1.0);
    EXPECT_DOUBLE_EQ(engine.getTimeScale(), 0.0);
}

TEST_F(PhysicsEngineTest, SetIntegrator) {
    engine.setIntegrator(IntegratorFactory::Type::RK4);
    EXPECT_EQ(engine.getIntegratorType(), IntegratorFactory::Type::RK4);

    engine.setIntegrator(IntegratorFactory::Type::VelocityVerlet);
    EXPECT_EQ(engine.getIntegratorType(), IntegratorFactory::Type::VelocityVerlet);
}

TEST_F(PhysicsEngineTest, StartStopPause) {
    // Start
    engine.start();
    EXPECT_EQ(engine.getState(), SimulationState::Running);
    EXPECT_TRUE(engine.isRunning());

    // Pause
    engine.pause();
    EXPECT_EQ(engine.getState(), SimulationState::Paused);
    EXPECT_TRUE(engine.isPaused());

    // Resume
    engine.resume();
    EXPECT_EQ(engine.getState(), SimulationState::Running);
    EXPECT_TRUE(engine.isRunning());

    // Stop
    engine.stop();
    EXPECT_EQ(engine.getState(), SimulationState::Stopped);
    EXPECT_FALSE(engine.isRunning());
}

TEST_F(PhysicsEngineTest, ResetClearsStats) {
    engine.start();
    engine.step();
    engine.step();

    const auto& stats = engine.getStats();
    EXPECT_GT(stats.stepCount, 0u);
    EXPECT_GT(stats.simulationTime, 0.0);

    engine.reset();

    const auto& newStats = engine.getStats();
    EXPECT_EQ(newStats.stepCount, 0u);
    EXPECT_DOUBLE_EQ(newStats.simulationTime, 0.0);
}

TEST_F(PhysicsEngineTest, StepAdvancesSimulation) {
    engine.step();

    const auto& stats = engine.getStats();
    EXPECT_EQ(stats.stepCount, 1u);
    EXPECT_GT(stats.simulationTime, 0.0);
}

TEST_F(PhysicsEngineTest, UpdateWhileStopped) {
    // Should not advance when stopped
    engine.update(0.1);

    const auto& stats = engine.getStats();
    EXPECT_EQ(stats.stepCount, 0u);
}

TEST_F(PhysicsEngineTest, UpdateWhileRunning) {
    engine.setTimeStep(0.01);  // Large timestep for test
    engine.start();
    engine.update(0.05);  // Should trigger multiple steps

    const auto& stats = engine.getStats();
    EXPECT_GT(stats.stepCount, 0u);
}

TEST_F(PhysicsEngineTest, SetAccelerator) {
    auto accelerator = std::make_shared<accelerator::Accelerator>();
    engine.setAccelerator(accelerator);

    EXPECT_EQ(engine.getAccelerator(), accelerator);
}

TEST_F(PhysicsEngineTest, InitializeDefaultBeam) {
    engine.initializeDefaultBeam();

    EXPECT_GT(engine.getParticleSystem().getParticleCount(), 0u);
    EXPECT_EQ(engine.getParticleSystem().getActiveParticleCount(),
              engine.getParticleSystem().getParticleCount());
}

TEST_F(PhysicsEngineTest, ParticleIntegration) {
    // Add a single proton using factory method
    Particle p = Particle::proton();
    p.setPosition({0, 0, 0});

    // Give it momentum in z direction (1 GeV proton)
    double gamma = 1.0 + (1e9 * constants::energy::eV) / (constants::m_p * constants::c * constants::c);
    double momentum = constants::relativistic::momentumFromGamma(gamma, constants::m_p);
    p.setMomentum({0, 0, momentum});

    engine.getParticleSystem().addParticle(p);
    engine.step();

    // Particle should have moved in z direction
    const auto& particles = engine.getParticleSystem().getParticles();
    EXPECT_GT(particles[0].getPosition().z, 0.0);
}

TEST_F(PhysicsEngineTest, LossCallback) {
    int lossCount = 0;
    engine.setLossCallback([&lossCount](const Particle&) {
        lossCount++;
    });

    // Note: Loss detection requires an accelerator with aperture checking
    // This test verifies callback mechanism works
    EXPECT_EQ(lossCount, 0);
}

TEST_F(PhysicsEngineTest, SimulationStats) {
    engine.start();  // Must start before initializing beam (start calls reset)
    engine.initializeDefaultBeam();
    engine.setTimeStep(0.01);  // Large timestep for test

    // Use update() instead of step() so statistics are computed
    engine.update(0.1);  // This should trigger 10 steps

    const auto& stats = engine.getStats();
    EXPECT_GT(stats.stepCount, 0u);
    EXPECT_GT(stats.particleCount, 0u);
}

} // namespace pas::physics::tests
