#include "physics/Integrator.hpp"
#include "physics/Constants.hpp"
#include <cmath>
#include <algorithm>

namespace pas::physics {

using namespace constants;

// EulerIntegrator implementation

void EulerIntegrator::step(Particle& particle,
                           const EMFieldManager& fieldManager,
                           double time,
                           double dt) {
    if (!particle.isActive()) return;

    glm::dvec3 pos = particle.getPosition();
    glm::dvec3 mom = particle.getMomentum();

    // Evaluate field at current position
    FieldValue field = fieldManager.evaluate(pos, time);

    // Calculate velocity from momentum
    glm::dvec3 vel = particle.getVelocity();

    // Lorentz force: F = q(E + v x B)
    double q = particle.getCharge();
    glm::dvec3 force = q * (field.E + glm::cross(vel, field.B));

    // Update momentum: dp = F * dt
    glm::dvec3 newMom = mom + force * dt;
    particle.setMomentum(newMom);

    // Update position: dx = v * dt (using new velocity)
    glm::dvec3 newVel = particle.getVelocity();
    glm::dvec3 newPos = pos + newVel * dt;
    particle.setPosition(newPos);
}

// VelocityVerletIntegrator implementation

void VelocityVerletIntegrator::step(Particle& particle,
                                    const EMFieldManager& fieldManager,
                                    double time,
                                    double dt) {
    if (!particle.isActive()) return;

    glm::dvec3 pos = particle.getPosition();
    glm::dvec3 mom = particle.getMomentum();
    double q = particle.getCharge();
    double m = particle.getMass();

    // Evaluate field at current position
    FieldValue field = fieldManager.evaluate(pos, time);

    // Calculate current velocity and acceleration
    glm::dvec3 vel = particle.getVelocity();
    glm::dvec3 force = q * (field.E + glm::cross(vel, field.B));
    glm::dvec3 acc = force / (particle.getGamma() * m);

    // Half-step position update: x' = x + v*dt/2 + a*dt^2/4
    glm::dvec3 halfPos = pos + vel * (dt * 0.5);

    // Full momentum update using force at current position
    glm::dvec3 newMom = mom + force * dt;
    particle.setMomentum(newMom);

    // Calculate new velocity
    glm::dvec3 newVel = particle.getVelocity();

    // Complete position update: x'' = x' + v_new*dt/2
    glm::dvec3 newPos = halfPos + newVel * (dt * 0.5);
    particle.setPosition(newPos);
}

// BorisIntegrator implementation

void BorisIntegrator::step(Particle& particle,
                           const EMFieldManager& fieldManager,
                           double time,
                           double dt) {
    if (!particle.isActive()) return;

    glm::dvec3 pos = particle.getPosition();
    glm::dvec3 mom = particle.getMomentum();
    double q = particle.getCharge();
    double m = particle.getMass();

    // Evaluate field at current position
    FieldValue field = fieldManager.evaluate(pos, time);

    // Half-step electric push
    glm::dvec3 momMinus = mom + q * field.E * (dt * 0.5);

    // Calculate gamma for magnetic rotation
    double pMag = glm::length(momMinus);
    double gamma = std::sqrt(1.0 + (pMag / (m * c)) * (pMag / (m * c)));

    // Boris rotation vectors
    glm::dvec3 t = (q * field.B * dt) / (2.0 * gamma * m);
    double tMag2 = glm::dot(t, t);
    glm::dvec3 s = (2.0 * t) / (1.0 + tMag2);

    // Calculate velocity for rotation (u = p / (gamma * m))
    glm::dvec3 uMinus = momMinus / (gamma * m);

    // Boris rotation
    glm::dvec3 uPrime = uMinus + glm::cross(uMinus, t);
    glm::dvec3 uPlus = uMinus + glm::cross(uPrime, s);

    // Convert back to momentum
    glm::dvec3 momPlus = uPlus * gamma * m;

    // Second half-step electric push
    glm::dvec3 newMom = momPlus + q * field.E * (dt * 0.5);
    particle.setMomentum(newMom);

    // Update position using average velocity
    glm::dvec3 newVel = particle.getVelocity();
    glm::dvec3 newPos = pos + newVel * dt;
    particle.setPosition(newPos);
}

// RK4Integrator implementation

RK4Integrator::State RK4Integrator::evaluateDerivative(
    const Particle& particle,
    const glm::dvec3& position,
    const glm::dvec3& momentum,
    const EMFieldManager& fieldManager,
    double time) const {

    double m = particle.getMass();
    double q = particle.getCharge();

    // Calculate gamma from momentum
    double pMag = glm::length(momentum);
    double gamma = std::sqrt(1.0 + (pMag / (m * c)) * (pMag / (m * c)));

    // Velocity from momentum
    glm::dvec3 vel = momentum / (gamma * m);

    // Evaluate field
    FieldValue field = fieldManager.evaluate(position, time);

    // Lorentz force
    glm::dvec3 force = q * (field.E + glm::cross(vel, field.B));

    return State{vel, force};
}

void RK4Integrator::step(Particle& particle,
                         const EMFieldManager& fieldManager,
                         double time,
                         double dt) {
    if (!particle.isActive()) return;

    glm::dvec3 pos = particle.getPosition();
    glm::dvec3 mom = particle.getMomentum();

    // k1
    State k1 = evaluateDerivative(particle, pos, mom, fieldManager, time);

    // k2
    glm::dvec3 pos2 = pos + k1.position * (dt * 0.5);
    glm::dvec3 mom2 = mom + k1.momentum * (dt * 0.5);
    State k2 = evaluateDerivative(particle, pos2, mom2, fieldManager, time + dt * 0.5);

    // k3
    glm::dvec3 pos3 = pos + k2.position * (dt * 0.5);
    glm::dvec3 mom3 = mom + k2.momentum * (dt * 0.5);
    State k3 = evaluateDerivative(particle, pos3, mom3, fieldManager, time + dt * 0.5);

    // k4
    glm::dvec3 pos4 = pos + k3.position * dt;
    glm::dvec3 mom4 = mom + k3.momentum * dt;
    State k4 = evaluateDerivative(particle, pos4, mom4, fieldManager, time + dt);

    // Combine
    glm::dvec3 newPos = pos + (k1.position + 2.0 * k2.position + 2.0 * k3.position + k4.position) * (dt / 6.0);
    glm::dvec3 newMom = mom + (k1.momentum + 2.0 * k2.momentum + 2.0 * k3.momentum + k4.momentum) * (dt / 6.0);

    particle.setPosition(newPos);
    particle.setMomentum(newMom);
}

// IntegratorFactory implementation

std::unique_ptr<Integrator> IntegratorFactory::create(Type type) {
    switch (type) {
        case Type::Euler:
            return std::make_unique<EulerIntegrator>();
        case Type::VelocityVerlet:
            return std::make_unique<VelocityVerletIntegrator>();
        case Type::Boris:
            return std::make_unique<BorisIntegrator>();
        case Type::RK4:
            return std::make_unique<RK4Integrator>();
        default:
            return std::make_unique<BorisIntegrator>();
    }
}

std::unique_ptr<Integrator> IntegratorFactory::create(const std::string& name) {
    if (name == "Euler") {
        return create(Type::Euler);
    } else if (name == "Verlet" || name == "VelocityVerlet") {
        return create(Type::VelocityVerlet);
    } else if (name == "Boris") {
        return create(Type::Boris);
    } else if (name == "RK4") {
        return create(Type::RK4);
    }
    // Default to Boris
    return create(Type::Boris);
}

} // namespace pas::physics
