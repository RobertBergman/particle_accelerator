#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>

#include "physics/EMField.hpp"

namespace pas::accelerator {

/**
 * @brief Types of accelerator components.
 */
enum class ComponentType {
    BeamPipe,
    Dipole,
    Quadrupole,
    Sextupole,
    RFCavity,
    Detector,
    Custom
};

/**
 * @brief Convert component type to string.
 */
std::string componentTypeToString(ComponentType type);

/**
 * @brief Aperture shape for beam pipe and magnets.
 */
enum class ApertureShape {
    Circular,
    Elliptical,
    Rectangular
};

/**
 * @brief Aperture parameters.
 */
struct Aperture {
    ApertureShape shape = ApertureShape::Circular;
    double radiusX = 0.05;  // meters (half-width for rectangular)
    double radiusY = 0.05;  // meters (half-height for rectangular)

    /**
     * @brief Check if a local position is within the aperture.
     */
    bool isInside(double x, double y) const;
};

/**
 * @brief Abstract base class for all accelerator components.
 *
 * Components represent physical beamline elements like magnets, cavities, etc.
 * Each component has a position, orientation, length, and aperture.
 */
class Component {
public:
    /**
     * @brief Construct a component.
     * @param name Unique name for this component.
     * @param length Effective length in meters.
     * @param aperture Aperture specification.
     */
    Component(std::string name, double length, const Aperture& aperture = Aperture());

    virtual ~Component() = default;

    // Type identification
    virtual ComponentType getType() const = 0;
    virtual std::string getTypeName() const { return componentTypeToString(getType()); }

    // Field access
    virtual std::shared_ptr<physics::FieldSource> getFieldSource() const = 0;

    // Geometry
    const std::string& getName() const { return m_name; }
    double getLength() const { return m_length; }
    const Aperture& getAperture() const { return m_aperture; }

    /**
     * @brief Get the s-position (longitudinal position along beamline).
     */
    double getSPosition() const { return m_sPosition; }

    /**
     * @brief Set the s-position (called by Accelerator during lattice construction).
     */
    void setSPosition(double s) { m_sPosition = s; }

    /**
     * @brief Get the entrance s-position.
     */
    double getEntranceS() const { return m_sPosition; }

    /**
     * @brief Get the exit s-position.
     */
    double getExitS() const { return m_sPosition + m_length; }

    // Position and orientation in global coordinates
    const glm::dvec3& getPosition() const { return m_position; }
    void setPosition(const glm::dvec3& pos) { m_position = pos; }

    const glm::dquat& getRotation() const { return m_rotation; }
    void setRotation(const glm::dquat& rot) { m_rotation = rot; }

    /**
     * @brief Transform a global position to local component coordinates.
     */
    glm::dvec3 toLocal(const glm::dvec3& globalPos) const;

    /**
     * @brief Transform a local position to global coordinates.
     */
    glm::dvec3 toGlobal(const glm::dvec3& localPos) const;

    /**
     * @brief Check if a global position is inside this component's aperture.
     */
    bool isInsideAperture(const glm::dvec3& globalPos) const;

    /**
     * @brief Check if an s-coordinate is within this component.
     */
    bool containsS(double s) const {
        return s >= m_sPosition && s < m_sPosition + m_length;
    }

protected:
    std::string m_name;
    double m_length;
    Aperture m_aperture;
    double m_sPosition = 0.0;
    glm::dvec3 m_position{0.0};
    glm::dquat m_rotation{1.0, 0.0, 0.0, 0.0};  // Identity quaternion
};

/**
 * @brief Beam pipe (drift space).
 *
 * No fields, just defines the vacuum chamber aperture.
 */
class BeamPipe : public Component {
public:
    BeamPipe(const std::string& name, double length, const Aperture& aperture = Aperture());

    ComponentType getType() const override { return ComponentType::BeamPipe; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override { return nullptr; }
};

/**
 * @brief Dipole magnet for beam bending.
 */
class Dipole : public Component {
public:
    /**
     * @brief Construct a dipole magnet.
     * @param name Component name.
     * @param length Effective length in meters.
     * @param field Magnetic field strength in Tesla (vertical by default).
     * @param aperture Aperture specification.
     */
    Dipole(const std::string& name, double length, double field,
           const Aperture& aperture = Aperture());

    ComponentType getType() const override { return ComponentType::Dipole; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getField() const { return m_field; }
    void setField(double field);

    /**
     * @brief Calculate the bending angle for a given momentum.
     * @param momentum Reference momentum in kg*m/s.
     * @return Bending angle in radians.
     */
    double getBendingAngle(double momentum) const;

    /**
     * @brief Calculate the bending radius for a given momentum.
     */
    double getBendingRadius(double momentum) const;

private:
    double m_field;  // Tesla
    mutable std::shared_ptr<physics::UniformBField> m_fieldSource;
};

/**
 * @brief Quadrupole magnet for focusing/defocusing.
 */
class Quadrupole : public Component {
public:
    /**
     * @brief Construct a quadrupole magnet.
     * @param name Component name.
     * @param length Effective length in meters.
     * @param gradient Field gradient in T/m. Positive = horizontal focusing.
     * @param aperture Aperture specification.
     */
    Quadrupole(const std::string& name, double length, double gradient,
               const Aperture& aperture = Aperture());

    ComponentType getType() const override { return ComponentType::Quadrupole; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getGradient() const { return m_gradient; }
    void setGradient(double gradient);

    /**
     * @brief Calculate K1 strength (normalized gradient).
     * @param momentum Reference momentum in kg*m/s.
     * @return K1 in m^-2.
     */
    double getK1(double momentum) const;

    /**
     * @brief Check if this is a focusing quadrupole (for horizontal plane).
     */
    bool isFocusing() const { return m_gradient > 0; }

private:
    double m_gradient;  // T/m
    mutable std::shared_ptr<physics::QuadrupoleField> m_fieldSource;
};

/**
 * @brief RF cavity for particle acceleration.
 */
class RFCavity : public Component {
public:
    /**
     * @brief Construct an RF cavity.
     * @param name Component name.
     * @param length Cavity length in meters.
     * @param voltage Peak voltage in Volts.
     * @param frequency RF frequency in Hz.
     * @param phase Synchronous phase in radians.
     * @param aperture Aperture specification.
     */
    RFCavity(const std::string& name, double length, double voltage,
             double frequency, double phase = 0.0,
             const Aperture& aperture = Aperture());

    ComponentType getType() const override { return ComponentType::RFCavity; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getVoltage() const { return m_voltage; }
    void setVoltage(double voltage);

    double getFrequency() const { return m_frequency; }
    void setFrequency(double frequency);

    double getPhase() const { return m_phase; }
    void setPhase(double phase);

    /**
     * @brief Calculate the energy gain per pass.
     * @param phase Particle phase relative to RF (radians).
     * @return Energy gain in Joules.
     */
    double getEnergyGain(double phase) const;

private:
    double m_voltage;    // Volts
    double m_frequency;  // Hz
    double m_phase;      // radians
    mutable std::shared_ptr<physics::RFField> m_fieldSource;
};

/**
 * @brief Detector for recording particle passage.
 */
class Detector : public Component {
public:
    /**
     * @brief Hit record for a particle passing through the detector.
     */
    struct Hit {
        double time;
        glm::dvec3 position;
        glm::dvec3 momentum;
        uint64_t particleId;
    };

    /**
     * @brief Construct a detector.
     * @param name Component name.
     * @param aperture Aperture specification.
     */
    Detector(const std::string& name, const Aperture& aperture = Aperture());

    ComponentType getType() const override { return ComponentType::Detector; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override { return nullptr; }

    /**
     * @brief Record a particle hit.
     */
    void recordHit(double time, const glm::dvec3& position,
                   const glm::dvec3& momentum, uint64_t particleId);

    /**
     * @brief Get all recorded hits.
     */
    const std::vector<Hit>& getHits() const { return m_hits; }

    /**
     * @brief Clear all hits.
     */
    void clearHits() { m_hits.clear(); }

    /**
     * @brief Get the number of hits.
     */
    size_t getHitCount() const { return m_hits.size(); }

private:
    std::vector<Hit> m_hits;
};

} // namespace pas::accelerator
