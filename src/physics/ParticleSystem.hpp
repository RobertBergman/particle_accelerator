#pragma once

#include "physics/Particle.hpp"
#include <vector>
#include <random>
#include <cstdint>

namespace pas::physics {

/**
 * @brief Parameters for beam generation.
 */
struct BeamParameters {
    // Particle type
    enum class ParticleType {
        Electron,
        Positron,
        Proton,
        Antiproton
    };
    ParticleType particleType = ParticleType::Proton;

    // Number of particles
    size_t numParticles = 1000;

    // Energy
    double kineticEnergy = 1e9 * constants::energy::eV;  // 1 GeV default

    // Spatial distribution (Gaussian sigma values)
    double sigmaX = 1e-3;   // m
    double sigmaY = 1e-3;   // m
    double sigmaZ = 1e-2;   // m (bunch length)

    // Momentum distribution (Gaussian sigma values relative to mean)
    double sigmaPx = 1e-4;  // Relative
    double sigmaPy = 1e-4;  // Relative
    double sigmaDelta = 1e-3;  // Energy spread

    // Initial position offset
    glm::dvec3 positionOffset{0.0};

    // Initial direction
    glm::dvec3 direction{0.0, 0.0, 1.0};  // +Z by default

    // Distribution type
    enum class Distribution {
        Gaussian,
        Uniform,
        Waterbag
    };
    Distribution distribution = Distribution::Gaussian;

    // Random seed for reproducibility
    uint64_t seed = 42;
};

/**
 * @brief Statistics for a particle beam.
 */
struct BeamStatistics {
    // Counts
    size_t totalParticles = 0;
    size_t activeParticles = 0;
    size_t lostParticles = 0;

    // Position statistics
    glm::dvec3 meanPosition{0.0};
    glm::dvec3 rmsSize{0.0};

    // Momentum statistics
    glm::dvec3 meanMomentum{0.0};
    glm::dvec3 rmsMomentum{0.0};

    // Energy statistics
    double meanEnergy = 0.0;
    double rmsEnergy = 0.0;
    double minEnergy = 0.0;
    double maxEnergy = 0.0;

    // Emittance (geometric, unnormalized)
    double emittanceX = 0.0;
    double emittanceY = 0.0;

    // Normalized emittance
    double normalizedEmittanceX = 0.0;
    double normalizedEmittanceY = 0.0;
};

/**
 * @brief Container for managing a collection of particles.
 *
 * Provides beam generation, statistics computation, and particle management.
 */
class ParticleSystem {
public:
    ParticleSystem();

    /**
     * @brief Generate a beam with the given parameters.
     * @param params Beam parameters.
     */
    void generateBeam(const BeamParameters& params);

    /**
     * @brief Clear all particles.
     */
    void clear();

    /**
     * @brief Add a particle to the system.
     */
    void addParticle(const Particle& particle);

    /**
     * @brief Remove inactive particles from the system.
     */
    void removeInactiveParticles();

    /**
     * @brief Get the number of particles.
     */
    size_t getParticleCount() const { return m_particles.size(); }

    /**
     * @brief Get the number of active particles.
     */
    size_t getActiveParticleCount() const;

    /**
     * @brief Get read-only access to all particles.
     */
    const std::vector<Particle>& getParticles() const { return m_particles; }

    /**
     * @brief Get mutable access to all particles.
     */
    std::vector<Particle>& getParticles() { return m_particles; }

    /**
     * @brief Get a specific particle.
     */
    Particle& getParticle(size_t index) { return m_particles[index]; }
    const Particle& getParticle(size_t index) const { return m_particles[index]; }

    /**
     * @brief Compute beam statistics.
     */
    BeamStatistics computeStatistics() const;

    /**
     * @brief Get the reference momentum for beam coordinates.
     */
    double getReferenceMomentum() const { return m_referenceMomentum; }

    /**
     * @brief Set the reference momentum.
     */
    void setReferenceMomentum(double momentum) { m_referenceMomentum = momentum; }

    /**
     * @brief Check if a particle is within a given aperture.
     * @param particle The particle to check.
     * @param radius Aperture radius in meters.
     * @return True if particle is within aperture.
     */
    static bool isWithinAperture(const Particle& particle, double radius);

    /**
     * @brief Mark particles outside aperture as inactive.
     * @param radius Aperture radius in meters.
     * @return Number of particles lost.
     */
    size_t applyAperture(double radius);

private:
    /**
     * @brief Create a particle of the specified type.
     */
    static Particle createParticle(BeamParameters::ParticleType type);

    std::vector<Particle> m_particles;
    double m_referenceMomentum;
    std::mt19937_64 m_rng;
};

} // namespace pas::physics
