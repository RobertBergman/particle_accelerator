#include <gtest/gtest.h>
#include <cmath>

#include "physics/Integrator.hpp"
#include "physics/Constants.hpp"

namespace pas::physics::tests {

using namespace constants;

class IntegratorTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-10;

    EMFieldManager emptyField;
};

// Factory tests

TEST_F(IntegratorTest, FactoryCreatesEuler) {
    auto integrator = IntegratorFactory::create("Euler");
    EXPECT_EQ(integrator->getName(), "Euler");
    EXPECT_EQ(integrator->getOrder(), 1);
}

TEST_F(IntegratorTest, FactoryCreatesVerlet) {
    auto integrator = IntegratorFactory::create("Verlet");
    EXPECT_EQ(integrator->getName(), "Velocity Verlet");
    EXPECT_EQ(integrator->getOrder(), 2);
}

TEST_F(IntegratorTest, FactoryCreateBoris) {
    auto integrator = IntegratorFactory::create("Boris");
    EXPECT_EQ(integrator->getName(), "Boris");
    EXPECT_EQ(integrator->getOrder(), 2);
}

TEST_F(IntegratorTest, FactoryCreatesRK4) {
    auto integrator = IntegratorFactory::create("RK4");
    EXPECT_EQ(integrator->getName(), "RK4");
    EXPECT_EQ(integrator->getOrder(), 4);
}

TEST_F(IntegratorTest, DefaultsToBoris) {
    auto integrator = IntegratorFactory::create("unknown");
    EXPECT_EQ(integrator->getName(), "Boris");
}

// Drift motion tests (no fields)

TEST_F(IntegratorTest, EulerDriftMotion) {
    Particle p = Particle::proton();
    p.setKineticEnergy(1.0 * energy::MeV);  // Give it some energy

    glm::dvec3 initialVel = p.getVelocity();
    glm::dvec3 initialPos = p.getPosition();
    double dt = 1e-9;

    EulerIntegrator integrator;
    integrator.step(p, emptyField, 0.0, dt);

    // Position should advance by v * dt
    glm::dvec3 expectedPos = initialPos + initialVel * dt;
    glm::dvec3 actualPos = p.getPosition();

    EXPECT_NEAR(actualPos.x, expectedPos.x, 1e-20);
    EXPECT_NEAR(actualPos.y, expectedPos.y, 1e-20);
    EXPECT_NEAR(actualPos.z, expectedPos.z, 1e-15);
}

TEST_F(IntegratorTest, BorisDriftMotion) {
    Particle p = Particle::proton();
    p.setKineticEnergy(1.0 * energy::MeV);

    glm::dvec3 initialVel = p.getVelocity();
    glm::dvec3 initialPos = p.getPosition();
    double dt = 1e-9;

    BorisIntegrator integrator;
    integrator.step(p, emptyField, 0.0, dt);

    glm::dvec3 expectedPos = initialPos + initialVel * dt;
    glm::dvec3 actualPos = p.getPosition();

    EXPECT_NEAR(actualPos.z, expectedPos.z, expectedPos.z * 1e-10);
}

TEST_F(IntegratorTest, RK4DriftMotion) {
    Particle p = Particle::proton();
    p.setKineticEnergy(1.0 * energy::MeV);

    glm::dvec3 initialVel = p.getVelocity();
    glm::dvec3 initialPos = p.getPosition();
    double dt = 1e-9;

    RK4Integrator integrator;
    integrator.step(p, emptyField, 0.0, dt);

    glm::dvec3 expectedPos = initialPos + initialVel * dt;
    glm::dvec3 actualPos = p.getPosition();

    EXPECT_NEAR(actualPos.z, expectedPos.z, expectedPos.z * 1e-10);
}

// Energy conservation in magnetic field

TEST_F(IntegratorTest, BorisConservesEnergyInBField) {
    Particle p = Particle::proton();
    p.setKineticEnergy(10.0 * energy::MeV);

    double initialEnergy = p.getKineticEnergy();

    // Uniform magnetic field (no energy change expected)
    EMFieldManager manager;
    manager.addSource(std::make_shared<UniformBField>(glm::dvec3(0.0, 0.0, 1.0)));

    BorisIntegrator integrator;
    double dt = 1e-12;

    // Run for many steps
    for (int i = 0; i < 10000; ++i) {
        integrator.step(p, manager, i * dt, dt);
    }

    double finalEnergy = p.getKineticEnergy();
    double relativeError = std::abs(finalEnergy - initialEnergy) / initialEnergy;

    // Boris should conserve energy very well
    EXPECT_LT(relativeError, 1e-10);
}

TEST_F(IntegratorTest, RK4ConservesEnergyInBField) {
    Particle p = Particle::proton();
    p.setKineticEnergy(10.0 * energy::MeV);

    double initialEnergy = p.getKineticEnergy();

    EMFieldManager manager;
    manager.addSource(std::make_shared<UniformBField>(glm::dvec3(0.0, 0.0, 1.0)));

    RK4Integrator integrator;
    double dt = 1e-12;

    for (int i = 0; i < 1000; ++i) {
        integrator.step(p, manager, i * dt, dt);
    }

    double finalEnergy = p.getKineticEnergy();
    double relativeError = std::abs(finalEnergy - initialEnergy) / initialEnergy;

    EXPECT_LT(relativeError, 1e-6);
}

// Cyclotron motion test

TEST_F(IntegratorTest, BorisCyclotronRadius) {
    Particle p = Particle::proton();

    // Set horizontal velocity
    double speed = 0.1 * c;  // 10% of c
    p.setVelocity(glm::dvec3(speed, 0.0, 0.0));

    double gamma = p.getGamma();
    double momentum = p.getMomentumMagnitude();

    // Vertical magnetic field
    double B = 1.0;  // 1 Tesla
    EMFieldManager manager;
    manager.addSource(std::make_shared<UniformBField>(glm::dvec3(0.0, 0.0, B)));

    // Theoretical cyclotron radius: r = p / (q * B)
    double theoreticalRadius = momentum / (std::abs(p.getCharge()) * B);

    BorisIntegrator integrator;

    // Use smaller time step for accuracy
    double cyclotronPeriod = 2.0 * constants::pi * gamma * p.getMass() / (std::abs(p.getCharge()) * B);
    double dt = cyclotronPeriod / 1000.0;  // 1000 steps per revolution

    // Run for a full period and check that particle returns near origin
    int steps = 1000;
    for (int i = 0; i < steps; ++i) {
        integrator.step(p, manager, i * dt, dt);
    }

    // After full revolution, particle should return close to starting point
    double x = p.getX();
    double y = p.getY();
    double distFromOrigin = std::sqrt(x * x + y * y);

    // After full circle, should be back near (0,0) - allow 5% of radius as error
    EXPECT_LT(distFromOrigin, theoreticalRadius * 0.05);
}

// Inactive particle tests

TEST_F(IntegratorTest, InactiveParticleIsNotUpdated) {
    Particle p = Particle::proton();
    p.setKineticEnergy(1.0 * energy::MeV);
    p.setActive(false);

    glm::dvec3 initialPos = p.getPosition();

    BorisIntegrator integrator;
    integrator.step(p, emptyField, 0.0, 1e-9);

    // Position should not change
    EXPECT_DOUBLE_EQ(p.getX(), initialPos.x);
    EXPECT_DOUBLE_EQ(p.getY(), initialPos.y);
    EXPECT_DOUBLE_EQ(p.getZ(), initialPos.z);
}

// Electric field acceleration

TEST_F(IntegratorTest, BorisAcceleratesInEField) {
    Particle p = Particle::proton();
    glm::dvec3 initialMom = p.getMomentum();
    double initialEnergy = p.getKineticEnergy();

    // Add RF field for acceleration
    EMFieldManager manager;
    double voltage = 1e6;  // 1 MV
    double length = 1.0;
    // Phase = 0 gives maximum acceleration at t=0
    manager.addSource(std::make_shared<RFField>(voltage, 1e9, 0.0,
                                                  glm::dvec3(0.0), length, 1.0));

    BorisIntegrator integrator;
    double dt = 1e-12;

    // Small number of steps
    for (int i = 0; i < 100; ++i) {
        integrator.step(p, manager, i * dt, dt);
    }

    // Energy should increase (proton is positive, field is in +z)
    EXPECT_GT(p.getKineticEnergy(), initialEnergy);
}

} // namespace pas::physics::tests
