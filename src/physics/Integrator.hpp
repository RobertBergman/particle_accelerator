#pragma once

#include "physics/Particle.hpp"
#include "physics/EMField.hpp"
#include <memory>
#include <string>

namespace pas::physics {

/**
 * @brief Abstract interface for numerical integrators.
 *
 * Implements the Strategy Pattern - different integration methods
 * can be swapped at runtime.
 */
class Integrator {
public:
    virtual ~Integrator() = default;

    /**
     * @brief Advance a particle by one time step.
     * @param particle The particle to update (modified in place).
     * @param fieldManager Field sources to evaluate.
     * @param time Current simulation time.
     * @param dt Time step in seconds.
     */
    virtual void step(Particle& particle,
                      const EMFieldManager& fieldManager,
                      double time,
                      double dt) = 0;

    /**
     * @brief Get the name of this integrator.
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get the order of accuracy.
     */
    virtual int getOrder() const = 0;
};

/**
 * @brief Simple Euler integrator (1st order).
 *
 * For testing and comparison only - not recommended for production.
 */
class EulerIntegrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fieldManager,
              double time,
              double dt) override;

    std::string getName() const override { return "Euler"; }
    int getOrder() const override { return 1; }
};

/**
 * @brief Velocity Verlet integrator (2nd order, symplectic).
 *
 * Good for conservative systems, preserves energy well.
 */
class VelocityVerletIntegrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fieldManager,
              double time,
              double dt) override;

    std::string getName() const override { return "Velocity Verlet"; }
    int getOrder() const override { return 2; }
};

/**
 * @brief Boris pusher (2nd order, phase-space volume preserving).
 *
 * The de-facto standard for charged particles in magnetic fields.
 * Separates electric and magnetic field effects for better accuracy.
 *
 * Algorithm:
 * 1. Half electric push: p' = p + (q*E/2)*dt
 * 2. Magnetic rotation using Boris rotation formula
 * 3. Half electric push: p'' = p' + (q*E/2)*dt
 * 4. Position update: x = x + v*dt
 */
class BorisIntegrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fieldManager,
              double time,
              double dt) override;

    std::string getName() const override { return "Boris"; }
    int getOrder() const override { return 2; }
};

/**
 * @brief 4th order Runge-Kutta integrator.
 *
 * Higher accuracy at the cost of 4 field evaluations per step.
 */
class RK4Integrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fieldManager,
              double time,
              double dt) override;

    std::string getName() const override { return "RK4"; }
    int getOrder() const override { return 4; }

private:
    // Helper struct for RK4 state
    struct State {
        glm::dvec3 position;
        glm::dvec3 momentum;
    };

    State evaluateDerivative(const Particle& particle,
                             const glm::dvec3& position,
                             const glm::dvec3& momentum,
                             const EMFieldManager& fieldManager,
                             double time) const;
};

/**
 * @brief Factory for creating integrators by name.
 */
class IntegratorFactory {
public:
    enum class Type {
        Euler,
        VelocityVerlet,
        Boris,
        RK4
    };

    /**
     * @brief Create an integrator of the specified type.
     */
    static std::unique_ptr<Integrator> create(Type type);

    /**
     * @brief Create an integrator by name.
     * @param name One of: "Euler", "Verlet", "Boris", "RK4"
     */
    static std::unique_ptr<Integrator> create(const std::string& name);
};

} // namespace pas::physics
