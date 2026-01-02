#include "physics/Particle.hpp"
#include <cmath>

namespace pas::physics {

using namespace constants;

uint64_t Particle::s_nextId = 0;

Particle::Particle(double mass, double charge,
                   const glm::dvec3& position,
                   const glm::dvec3& momentum)
    : m_position(position)
    , m_momentum(momentum)
    , m_mass(mass)
    , m_charge(charge)
    , m_restEnergy(mass * c2)
    , m_gamma(1.0)
    , m_beta(0.0)
    , m_active(true)
    , m_id(s_nextId++) {
    updateDerivedQuantities();
}

Particle Particle::electron(const glm::dvec3& position, const glm::dvec3& momentum) {
    return Particle(m_e, -e, position, momentum);
}

Particle Particle::positron(const glm::dvec3& position, const glm::dvec3& momentum) {
    return Particle(m_e, e, position, momentum);
}

Particle Particle::proton(const glm::dvec3& position, const glm::dvec3& momentum) {
    return Particle(m_p, e, position, momentum);
}

Particle Particle::antiproton(const glm::dvec3& position, const glm::dvec3& momentum) {
    return Particle(m_p, -e, position, momentum);
}

void Particle::setMomentum(const glm::dvec3& momentum) {
    m_momentum = momentum;
    updateDerivedQuantities();
}

void Particle::setPx(double px) {
    m_momentum.x = px;
    updateDerivedQuantities();
}

void Particle::setPy(double py) {
    m_momentum.y = py;
    updateDerivedQuantities();
}

void Particle::setPz(double pz) {
    m_momentum.z = pz;
    updateDerivedQuantities();
}

double Particle::getMomentumMagnitude() const {
    return glm::length(m_momentum);
}

glm::dvec3 Particle::getVelocity() const {
    // v = p / (gamma * m)
    if (m_gamma > 0.0 && m_mass > 0.0) {
        return m_momentum / (m_gamma * m_mass);
    }
    return glm::dvec3(0.0);
}

void Particle::setVelocity(const glm::dvec3& velocity) {
    double speed = glm::length(velocity);
    if (speed >= c) {
        // Clamp to just below speed of light
        double scale = 0.999999 * c / speed;
        glm::dvec3 clampedVelocity = velocity * scale;
        speed = glm::length(clampedVelocity);
        m_beta = speed / c;
        m_gamma = relativistic::gammaFromBeta(m_beta);
        m_momentum = m_gamma * m_mass * clampedVelocity;
    } else if (speed > 0.0) {
        m_beta = speed / c;
        m_gamma = relativistic::gammaFromBeta(m_beta);
        m_momentum = m_gamma * m_mass * velocity;
    } else {
        m_beta = 0.0;
        m_gamma = 1.0;
        m_momentum = glm::dvec3(0.0);
    }
}

double Particle::getSpeed() const {
    return m_beta * c;
}

double Particle::getTotalEnergy() const {
    return m_gamma * m_restEnergy;
}

double Particle::getKineticEnergy() const {
    return (m_gamma - 1.0) * m_restEnergy;
}

void Particle::setKineticEnergy(double kineticEnergy, const glm::dvec3& direction) {
    // Calculate gamma from kinetic energy
    m_gamma = 1.0 + kineticEnergy / m_restEnergy;
    m_beta = relativistic::betaFromGamma(m_gamma);

    // Determine direction
    glm::dvec3 dir = direction;
    double dirLength = glm::length(dir);
    if (dirLength < 1e-10) {
        // Use current momentum direction if available, otherwise +z
        double currentMag = getMomentumMagnitude();
        if (currentMag > 1e-30) {
            dir = m_momentum / currentMag;
        } else {
            dir = glm::dvec3(0.0, 0.0, 1.0);
        }
    } else {
        dir = glm::normalize(dir);
    }

    // Calculate momentum magnitude: p = gamma * m * v = gamma * beta * m * c
    double momentumMag = m_gamma * m_beta * m_mass * c;
    m_momentum = dir * momentumMag;
}

double Particle::getDelta(double referenceMomentum) const {
    double p = getMomentumMagnitude();
    return (p - referenceMomentum) / referenceMomentum;
}

void Particle::updateDerivedQuantities() {
    double p = getMomentumMagnitude();
    if (p > 0.0 && m_mass > 0.0) {
        // gamma = sqrt(1 + (p/(m*c))^2)
        double pOverMc = p / (m_mass * c);
        m_gamma = std::sqrt(1.0 + pOverMc * pOverMc);
        m_beta = relativistic::betaFromGamma(m_gamma);
    } else {
        m_gamma = 1.0;
        m_beta = 0.0;
    }
}

} // namespace pas::physics
