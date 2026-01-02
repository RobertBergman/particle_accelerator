#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <limits>

namespace pas::physics {

/**
 * @brief Represents an electromagnetic field value at a point.
 */
struct FieldValue {
    glm::dvec3 E{0.0};  // Electric field (V/m)
    glm::dvec3 B{0.0};  // Magnetic field (Tesla)

    FieldValue() = default;
    FieldValue(const glm::dvec3& e, const glm::dvec3& b) : E(e), B(b) {}

    FieldValue& operator+=(const FieldValue& other) {
        E += other.E;
        B += other.B;
        return *this;
    }

    FieldValue operator+(const FieldValue& other) const {
        return FieldValue(E + other.E, B + other.B);
    }

    FieldValue operator*(double scalar) const {
        return FieldValue(E * scalar, B * scalar);
    }
};

/**
 * @brief Axis-aligned bounding box for spatial queries.
 */
struct BoundingBox {
    glm::dvec3 min{-std::numeric_limits<double>::infinity()};
    glm::dvec3 max{std::numeric_limits<double>::infinity()};

    BoundingBox() = default;
    BoundingBox(const glm::dvec3& minPt, const glm::dvec3& maxPt)
        : min(minPt), max(maxPt) {}

    bool contains(const glm::dvec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    bool isInfinite() const {
        return min.x == -std::numeric_limits<double>::infinity() ||
               max.x == std::numeric_limits<double>::infinity();
    }
};

/**
 * @brief Abstract interface for electromagnetic field sources.
 *
 * Follows the Open/Closed Principle - new field types can be added
 * without modifying existing code.
 */
class FieldSource {
public:
    virtual ~FieldSource() = default;

    /**
     * @brief Evaluate the field at a given position and time.
     * @param position Position in meters (global coordinates).
     * @param time Current simulation time in seconds.
     * @return FieldValue containing E and B vectors.
     */
    virtual FieldValue evaluate(const glm::dvec3& position, double time) const = 0;

    /**
     * @brief Get the bounding box of the field region.
     */
    virtual BoundingBox getBoundingBox() const = 0;

    /**
     * @brief Check if a position is within the active field region.
     */
    virtual bool isInside(const glm::dvec3& position) const {
        return getBoundingBox().contains(position);
    }

    /**
     * @brief Check if the field is enabled.
     */
    bool isEnabled() const { return m_enabled; }

    /**
     * @brief Enable or disable the field source.
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    bool m_enabled = true;
};

/**
 * @brief Composite container for multiple field sources.
 *
 * Implements the Composite Pattern to sum contributions from all sources.
 */
class EMFieldManager {
public:
    EMFieldManager() = default;

    /**
     * @brief Add a field source to the manager.
     */
    void addSource(std::shared_ptr<FieldSource> source);

    /**
     * @brief Remove a field source from the manager.
     */
    void removeSource(const std::shared_ptr<FieldSource>& source);

    /**
     * @brief Clear all field sources.
     */
    void clear();

    /**
     * @brief Get the number of field sources.
     */
    size_t getSourceCount() const { return m_sources.size(); }

    /**
     * @brief Evaluate the total field at a position and time.
     * @param position Position in meters.
     * @param time Simulation time in seconds.
     * @return Combined FieldValue from all enabled sources.
     */
    FieldValue evaluate(const glm::dvec3& position, double time) const;

    /**
     * @brief Get all field sources.
     */
    const std::vector<std::shared_ptr<FieldSource>>& getSources() const {
        return m_sources;
    }

private:
    std::vector<std::shared_ptr<FieldSource>> m_sources;
};

// Concrete field implementations

/**
 * @brief Uniform magnetic field (e.g., dipole magnet approximation).
 */
class UniformBField : public FieldSource {
public:
    /**
     * @brief Create a uniform magnetic field.
     * @param field Magnetic field vector in Tesla.
     * @param bounds Bounding box for the field region.
     */
    explicit UniformBField(const glm::dvec3& field,
                           const BoundingBox& bounds = BoundingBox());

    FieldValue evaluate(const glm::dvec3& position, double time) const override;
    BoundingBox getBoundingBox() const override { return m_bounds; }

    const glm::dvec3& getField() const { return m_field; }
    void setField(const glm::dvec3& field) { m_field = field; }

private:
    glm::dvec3 m_field;
    BoundingBox m_bounds;
};

/**
 * @brief Quadrupole magnetic field for focusing/defocusing.
 *
 * Field components: Bx = G * y, By = G * x, Bz = 0
 * where G is the gradient in T/m.
 */
class QuadrupoleField : public FieldSource {
public:
    /**
     * @brief Create a quadrupole field.
     * @param gradient Field gradient in T/m. Positive = horizontal focusing.
     * @param center Center position of the quadrupole.
     * @param length Effective length along z-axis.
     * @param aperture Radius of the aperture.
     */
    QuadrupoleField(double gradient,
                    const glm::dvec3& center = glm::dvec3(0.0),
                    double length = 1.0,
                    double aperture = 0.1);

    FieldValue evaluate(const glm::dvec3& position, double time) const override;
    BoundingBox getBoundingBox() const override { return m_bounds; }

    double getGradient() const { return m_gradient; }
    void setGradient(double gradient) { m_gradient = gradient; }

    double getAperture() const { return m_aperture; }

private:
    double m_gradient;    // T/m
    glm::dvec3 m_center;
    double m_length;
    double m_aperture;
    BoundingBox m_bounds;
};

/**
 * @brief RF cavity oscillating electric field for acceleration.
 *
 * E_z = V/L * cos(omega * t + phi)
 */
class RFField : public FieldSource {
public:
    /**
     * @brief Create an RF cavity field.
     * @param voltage Peak voltage in Volts.
     * @param frequency RF frequency in Hz.
     * @param phase Phase offset in radians.
     * @param center Center position of the cavity.
     * @param length Cavity length.
     * @param aperture Cavity aperture radius.
     */
    RFField(double voltage,
            double frequency,
            double phase = 0.0,
            const glm::dvec3& center = glm::dvec3(0.0),
            double length = 0.5,
            double aperture = 0.1);

    FieldValue evaluate(const glm::dvec3& position, double time) const override;
    BoundingBox getBoundingBox() const override { return m_bounds; }

    double getVoltage() const { return m_voltage; }
    void setVoltage(double voltage) { m_voltage = voltage; }

    double getFrequency() const { return m_frequency; }
    void setFrequency(double frequency);

    double getPhase() const { return m_phase; }
    void setPhase(double phase) { m_phase = phase; }

private:
    double m_voltage;     // V
    double m_frequency;   // Hz
    double m_omega;       // Angular frequency (rad/s)
    double m_phase;       // rad
    glm::dvec3 m_center;
    double m_length;
    double m_aperture;
    BoundingBox m_bounds;
};

} // namespace pas::physics
