#include "physics/ParticleSystem.hpp"
#include <algorithm>
#include <cmath>

namespace pas::physics {

using namespace constants;

ParticleSystem::ParticleSystem()
    : m_referenceMomentum(0.0)
    , m_rng(42) {
}

Particle ParticleSystem::createParticle(BeamParameters::ParticleType type) {
    switch (type) {
        case BeamParameters::ParticleType::Electron:
            return Particle::electron();
        case BeamParameters::ParticleType::Positron:
            return Particle::positron();
        case BeamParameters::ParticleType::Proton:
            return Particle::proton();
        case BeamParameters::ParticleType::Antiproton:
            return Particle::antiproton();
        default:
            return Particle::proton();
    }
}

void ParticleSystem::generateBeam(const BeamParameters& params) {
    clear();
    m_particles.reserve(params.numParticles);

    // Seed RNG
    m_rng.seed(params.seed);

    // Create distributions
    std::normal_distribution<double> normalDist(0.0, 1.0);
    std::uniform_real_distribution<double> uniformDist(-1.0, 1.0);

    // Create a reference particle to get mass
    Particle refParticle = createParticle(params.particleType);
    double mass = refParticle.getMass();

    // Calculate reference momentum from kinetic energy
    double gamma = relativistic::gammaFromKineticEnergy(params.kineticEnergy, mass);
    double beta = relativistic::betaFromGamma(gamma);
    double pRef = gamma * beta * mass * c;
    m_referenceMomentum = pRef;

    // Normalize direction
    glm::dvec3 dir = glm::normalize(params.direction);

    for (size_t i = 0; i < params.numParticles; ++i) {
        Particle particle = createParticle(params.particleType);

        // Generate position offset
        double dx, dy, dz;
        if (params.distribution == BeamParameters::Distribution::Gaussian) {
            dx = normalDist(m_rng) * params.sigmaX;
            dy = normalDist(m_rng) * params.sigmaY;
            dz = normalDist(m_rng) * params.sigmaZ;
        } else if (params.distribution == BeamParameters::Distribution::Uniform) {
            dx = uniformDist(m_rng) * params.sigmaX * std::sqrt(3.0);
            dy = uniformDist(m_rng) * params.sigmaY * std::sqrt(3.0);
            dz = uniformDist(m_rng) * params.sigmaZ * std::sqrt(3.0);
        } else {
            // Waterbag: uniform in 6D phase space
            double r = std::cbrt(std::abs(uniformDist(m_rng)));
            double theta = std::acos(uniformDist(m_rng));
            double phi = uniformDist(m_rng) * constants::pi;
            dx = r * std::sin(theta) * std::cos(phi) * params.sigmaX;
            dy = r * std::sin(theta) * std::sin(phi) * params.sigmaY;
            dz = r * std::cos(theta) * params.sigmaZ;
        }

        glm::dvec3 position = params.positionOffset + glm::dvec3(dx, dy, dz);
        particle.setPosition(position);

        // Generate momentum deviation
        double dpx, dpy, delta;
        if (params.distribution == BeamParameters::Distribution::Gaussian) {
            dpx = normalDist(m_rng) * params.sigmaPx;
            dpy = normalDist(m_rng) * params.sigmaPy;
            delta = normalDist(m_rng) * params.sigmaDelta;
        } else {
            dpx = uniformDist(m_rng) * params.sigmaPx * std::sqrt(3.0);
            dpy = uniformDist(m_rng) * params.sigmaPy * std::sqrt(3.0);
            delta = uniformDist(m_rng) * params.sigmaDelta * std::sqrt(3.0);
        }

        // Calculate actual momentum
        double pMag = pRef * (1.0 + delta);

        // Add transverse momentum components
        // The main momentum is along the direction, with small transverse deviations
        glm::dvec3 momentum = dir * pMag;

        // Create perpendicular vectors for transverse momentum
        glm::dvec3 perpX, perpY;
        if (std::abs(dir.y) < 0.9) {
            perpX = glm::normalize(glm::cross(dir, glm::dvec3(0, 1, 0)));
        } else {
            perpX = glm::normalize(glm::cross(dir, glm::dvec3(1, 0, 0)));
        }
        perpY = glm::cross(dir, perpX);

        momentum += perpX * (pRef * dpx) + perpY * (pRef * dpy);

        particle.setMomentum(momentum);
        m_particles.push_back(particle);
    }
}

void ParticleSystem::clear() {
    m_particles.clear();
}

void ParticleSystem::addParticle(const Particle& particle) {
    m_particles.push_back(particle);
}

void ParticleSystem::removeInactiveParticles() {
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
                       [](const Particle& p) { return !p.isActive(); }),
        m_particles.end()
    );
}

size_t ParticleSystem::getActiveParticleCount() const {
    return std::count_if(m_particles.begin(), m_particles.end(),
                         [](const Particle& p) { return p.isActive(); });
}

BeamStatistics ParticleSystem::computeStatistics() const {
    BeamStatistics stats;
    stats.totalParticles = m_particles.size();

    if (m_particles.empty()) {
        return stats;
    }

    // Count active particles
    std::vector<const Particle*> active;
    active.reserve(m_particles.size());
    for (const auto& p : m_particles) {
        if (p.isActive()) {
            active.push_back(&p);
        }
    }
    stats.activeParticles = active.size();
    stats.lostParticles = stats.totalParticles - stats.activeParticles;

    if (active.empty()) {
        return stats;
    }

    // Compute means
    glm::dvec3 sumPos(0.0);
    glm::dvec3 sumMom(0.0);
    double sumEnergy = 0.0;
    stats.minEnergy = active[0]->getKineticEnergy();
    stats.maxEnergy = stats.minEnergy;

    for (const Particle* p : active) {
        sumPos += p->getPosition();
        sumMom += p->getMomentum();
        double ke = p->getKineticEnergy();
        sumEnergy += ke;
        stats.minEnergy = std::min(stats.minEnergy, ke);
        stats.maxEnergy = std::max(stats.maxEnergy, ke);
    }

    double n = static_cast<double>(active.size());
    stats.meanPosition = sumPos / n;
    stats.meanMomentum = sumMom / n;
    stats.meanEnergy = sumEnergy / n;

    // Compute RMS values and emittance
    glm::dvec3 sumPosSq(0.0);
    glm::dvec3 sumMomSq(0.0);
    double sumEnergySq = 0.0;

    // For emittance: <x^2>, <x'^2>, <x*x'>
    double sumX2 = 0.0, sumXp2 = 0.0, sumXXp = 0.0;
    double sumY2 = 0.0, sumYp2 = 0.0, sumYYp = 0.0;

    for (const Particle* p : active) {
        glm::dvec3 dPos = p->getPosition() - stats.meanPosition;
        glm::dvec3 dMom = p->getMomentum() - stats.meanMomentum;
        double dEnergy = p->getKineticEnergy() - stats.meanEnergy;

        sumPosSq += dPos * dPos;
        sumMomSq += dMom * dMom;
        sumEnergySq += dEnergy * dEnergy;

        // Emittance calculation (x' = px/pz)
        double pz = p->getMomentum().z;
        if (std::abs(pz) > 1e-30) {
            double xp = p->getMomentum().x / pz;
            double yp = p->getMomentum().y / pz;

            sumX2 += dPos.x * dPos.x;
            sumXp2 += xp * xp;
            sumXXp += dPos.x * xp;

            sumY2 += dPos.y * dPos.y;
            sumYp2 += yp * yp;
            sumYYp += dPos.y * yp;
        }
    }

    stats.rmsSize = glm::sqrt(sumPosSq / n);
    stats.rmsMomentum = glm::sqrt(sumMomSq / n);
    stats.rmsEnergy = std::sqrt(sumEnergySq / n);

    // Geometric emittance: epsilon = sqrt(<x^2><x'^2> - <x*x'>^2)
    double avgX2 = sumX2 / n;
    double avgXp2 = sumXp2 / n;
    double avgXXp = sumXXp / n;
    stats.emittanceX = std::sqrt(std::max(0.0, avgX2 * avgXp2 - avgXXp * avgXXp));

    double avgY2 = sumY2 / n;
    double avgYp2 = sumYp2 / n;
    double avgYYp = sumYYp / n;
    stats.emittanceY = std::sqrt(std::max(0.0, avgY2 * avgYp2 - avgYYp * avgYYp));

    // Normalized emittance: epsilon_n = beta * gamma * epsilon
    double pRef = m_referenceMomentum;
    if (pRef > 0.0) {
        // Use first active particle's mass for calculation
        double mass = active[0]->getMass();
        double gamma = relativistic::gammaFromMomentum(pRef, mass);
        double beta = relativistic::betaFromGamma(gamma);
        double betaGamma = beta * gamma;

        stats.normalizedEmittanceX = betaGamma * stats.emittanceX;
        stats.normalizedEmittanceY = betaGamma * stats.emittanceY;
    }

    return stats;
}

bool ParticleSystem::isWithinAperture(const Particle& particle, double radius) {
    const glm::dvec3& pos = particle.getPosition();
    double r = std::sqrt(pos.x * pos.x + pos.y * pos.y);
    return r <= radius;
}

size_t ParticleSystem::applyAperture(double radius) {
    size_t lostCount = 0;
    for (auto& particle : m_particles) {
        if (particle.isActive() && !isWithinAperture(particle, radius)) {
            particle.setActive(false);
            ++lostCount;
        }
    }
    return lostCount;
}

} // namespace pas::physics
