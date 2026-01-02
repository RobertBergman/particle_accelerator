#pragma once

#include <glm/glm.hpp>
#include "physics/Constants.hpp"

namespace pas::physics {

/**
 * @brief Represents a charged particle in 6D phase space.
 *
 * Tracks position (x, y, z), momentum (px, py, pz), and particle properties
 * (mass, charge). Provides relativistic calculations for gamma, beta, and energy.
 */
class Particle {
public:
    /**
     * @brief Construct a particle with given properties.
     * @param mass Rest mass in kg.
     * @param charge Electric charge in Coulombs.
     * @param position Initial position in meters.
     * @param momentum Initial momentum in kg*m/s.
     */
    Particle(double mass, double charge,
             const glm::dvec3& position = glm::dvec3(0.0),
             const glm::dvec3& momentum = glm::dvec3(0.0));

    // Factory methods for common particle types

    /**
     * @brief Create an electron.
     */
    static Particle electron(const glm::dvec3& position = glm::dvec3(0.0),
                             const glm::dvec3& momentum = glm::dvec3(0.0));

    /**
     * @brief Create a positron.
     */
    static Particle positron(const glm::dvec3& position = glm::dvec3(0.0),
                             const glm::dvec3& momentum = glm::dvec3(0.0));

    /**
     * @brief Create a proton.
     */
    static Particle proton(const glm::dvec3& position = glm::dvec3(0.0),
                           const glm::dvec3& momentum = glm::dvec3(0.0));

    /**
     * @brief Create an antiproton.
     */
    static Particle antiproton(const glm::dvec3& position = glm::dvec3(0.0),
                               const glm::dvec3& momentum = glm::dvec3(0.0));

    // Position accessors
    const glm::dvec3& getPosition() const { return m_position; }
    void setPosition(const glm::dvec3& position) { m_position = position; }

    double getX() const { return m_position.x; }
    double getY() const { return m_position.y; }
    double getZ() const { return m_position.z; }

    void setX(double x) { m_position.x = x; }
    void setY(double y) { m_position.y = y; }
    void setZ(double z) { m_position.z = z; }

    // Momentum accessors
    const glm::dvec3& getMomentum() const { return m_momentum; }
    void setMomentum(const glm::dvec3& momentum);

    double getPx() const { return m_momentum.x; }
    double getPy() const { return m_momentum.y; }
    double getPz() const { return m_momentum.z; }

    void setPx(double px);
    void setPy(double py);
    void setPz(double pz);

    /**
     * @brief Get momentum magnitude.
     */
    double getMomentumMagnitude() const;

    // Velocity accessors (derived from momentum)

    /**
     * @brief Get velocity vector in m/s.
     */
    glm::dvec3 getVelocity() const;

    /**
     * @brief Set velocity and update momentum accordingly.
     */
    void setVelocity(const glm::dvec3& velocity);

    /**
     * @brief Get velocity magnitude in m/s.
     */
    double getSpeed() const;

    // Particle properties (immutable)
    double getMass() const { return m_mass; }
    double getCharge() const { return m_charge; }
    double getRestEnergy() const { return m_restEnergy; }

    // Relativistic quantities (derived)

    /**
     * @brief Get Lorentz factor gamma = 1/sqrt(1 - v^2/c^2).
     */
    double getGamma() const { return m_gamma; }

    /**
     * @brief Get beta = v/c.
     */
    double getBeta() const { return m_beta; }

    /**
     * @brief Get total energy E = gamma * m * c^2.
     */
    double getTotalEnergy() const;

    /**
     * @brief Get kinetic energy Ek = (gamma - 1) * m * c^2.
     */
    double getKineticEnergy() const;

    /**
     * @brief Set kinetic energy and update momentum accordingly.
     * @param kineticEnergy Kinetic energy in Joules.
     * @param direction Normalized direction vector (default: current direction or +z).
     */
    void setKineticEnergy(double kineticEnergy, const glm::dvec3& direction = glm::dvec3(0.0));

    // Beam coordinates (for lattice physics)

    /**
     * @brief Relative momentum deviation delta = (p - p0) / p0.
     */
    double getDelta(double referenceMomentum) const;

    // State management

    /**
     * @brief Check if particle is active (not lost).
     */
    bool isActive() const { return m_active; }

    /**
     * @brief Mark particle as lost/inactive.
     */
    void setActive(bool active) { m_active = active; }

    /**
     * @brief Get unique particle ID.
     */
    uint64_t getId() const { return m_id; }

private:
    /**
     * @brief Recalculate derived quantities (gamma, beta) from momentum.
     */
    void updateDerivedQuantities();

    static uint64_t s_nextId;

    // Phase space coordinates
    glm::dvec3 m_position;  // meters
    glm::dvec3 m_momentum;  // kg*m/s

    // Particle properties
    double m_mass;          // kg
    double m_charge;        // Coulombs
    double m_restEnergy;    // Joules

    // Derived relativistic quantities
    double m_gamma;         // Lorentz factor
    double m_beta;          // v/c

    // State
    bool m_active;
    uint64_t m_id;
};

} // namespace pas::physics
