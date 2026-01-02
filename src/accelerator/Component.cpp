#include "accelerator/Component.hpp"
#include "physics/Constants.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace pas::accelerator {

using namespace physics;
using namespace physics::constants;

std::string componentTypeToString(ComponentType type) {
    switch (type) {
        case ComponentType::BeamPipe:   return "BeamPipe";
        case ComponentType::Dipole:     return "Dipole";
        case ComponentType::Quadrupole: return "Quadrupole";
        case ComponentType::Sextupole:  return "Sextupole";
        case ComponentType::RFCavity:   return "RFCavity";
        case ComponentType::Detector:   return "Detector";
        case ComponentType::Custom:     return "Custom";
        default:                        return "Unknown";
    }
}

// Aperture implementation

bool Aperture::isInside(double x, double y) const {
    switch (shape) {
        case ApertureShape::Circular: {
            double r = std::sqrt(x * x + y * y);
            return r <= radiusX;
        }
        case ApertureShape::Elliptical: {
            double nx = x / radiusX;
            double ny = y / radiusY;
            return (nx * nx + ny * ny) <= 1.0;
        }
        case ApertureShape::Rectangular: {
            return std::abs(x) <= radiusX && std::abs(y) <= radiusY;
        }
        default:
            return true;
    }
}

// Component implementation

Component::Component(std::string name, double length, const Aperture& aperture)
    : m_name(std::move(name))
    , m_length(length)
    , m_aperture(aperture) {
}

glm::dvec3 Component::toLocal(const glm::dvec3& globalPos) const {
    // Translate then rotate
    glm::dvec3 translated = globalPos - m_position;
    return glm::inverse(m_rotation) * translated;
}

glm::dvec3 Component::toGlobal(const glm::dvec3& localPos) const {
    // Rotate then translate
    glm::dvec3 rotated = m_rotation * localPos;
    return rotated + m_position;
}

bool Component::isInsideAperture(const glm::dvec3& globalPos) const {
    glm::dvec3 local = toLocal(globalPos);

    // Check z is within length
    if (local.z < 0.0 || local.z > m_length) {
        return false;
    }

    // Check transverse aperture
    return m_aperture.isInside(local.x, local.y);
}

// BeamPipe implementation

BeamPipe::BeamPipe(const std::string& name, double length, const Aperture& aperture)
    : Component(name, length, aperture) {
}

// Dipole implementation

Dipole::Dipole(const std::string& name, double length, double field, const Aperture& aperture)
    : Component(name, length, aperture)
    , m_field(field) {
}

std::shared_ptr<FieldSource> Dipole::getFieldSource() const {
    if (!m_fieldSource) {
        // Vertical magnetic field (bends in horizontal plane)
        glm::dvec3 B(0.0, m_field, 0.0);

        // Calculate bounding box based on component position and length
        double halfLength = m_length / 2.0;
        double r = m_aperture.radiusX;
        BoundingBox bounds(
            glm::dvec3(m_position.x - r, m_position.y - r, m_position.z - halfLength),
            glm::dvec3(m_position.x + r, m_position.y + r, m_position.z + halfLength)
        );

        m_fieldSource = std::make_shared<UniformBField>(B, bounds);
    }
    return m_fieldSource;
}

void Dipole::setField(double field) {
    m_field = field;
    m_fieldSource.reset();  // Force recreation
}

double Dipole::getBendingAngle(double momentum) const {
    // theta = q * B * L / p
    return e * std::abs(m_field) * m_length / momentum;
}

double Dipole::getBendingRadius(double momentum) const {
    // rho = p / (q * B)
    if (std::abs(m_field) < 1e-10) {
        return std::numeric_limits<double>::infinity();
    }
    return momentum / (e * std::abs(m_field));
}

// Quadrupole implementation

Quadrupole::Quadrupole(const std::string& name, double length, double gradient,
                       const Aperture& aperture)
    : Component(name, length, aperture)
    , m_gradient(gradient) {
}

std::shared_ptr<FieldSource> Quadrupole::getFieldSource() const {
    if (!m_fieldSource) {
        m_fieldSource = std::make_shared<QuadrupoleField>(
            m_gradient,
            m_position,
            m_length,
            m_aperture.radiusX
        );
    }
    return m_fieldSource;
}

void Quadrupole::setGradient(double gradient) {
    m_gradient = gradient;
    m_fieldSource.reset();
}

double Quadrupole::getK1(double momentum) const {
    // K1 = (q * G) / p  [m^-2]
    return e * m_gradient / momentum;
}

// RFCavity implementation

RFCavity::RFCavity(const std::string& name, double length, double voltage,
                   double frequency, double phase, const Aperture& aperture)
    : Component(name, length, aperture)
    , m_voltage(voltage)
    , m_frequency(frequency)
    , m_phase(phase) {
}

std::shared_ptr<FieldSource> RFCavity::getFieldSource() const {
    if (!m_fieldSource) {
        m_fieldSource = std::make_shared<RFField>(
            m_voltage,
            m_frequency,
            m_phase,
            m_position,
            m_length,
            m_aperture.radiusX
        );
    }
    return m_fieldSource;
}

void RFCavity::setVoltage(double voltage) {
    m_voltage = voltage;
    m_fieldSource.reset();
}

void RFCavity::setFrequency(double frequency) {
    m_frequency = frequency;
    m_fieldSource.reset();
}

void RFCavity::setPhase(double phase) {
    m_phase = phase;
    m_fieldSource.reset();
}

double RFCavity::getEnergyGain(double phase) const {
    return e * m_voltage * std::cos(phase);
}

// Detector implementation

Detector::Detector(const std::string& name, const Aperture& aperture)
    : Component(name, 0.001, aperture) {  // Thin detector
}

void Detector::recordHit(double time, const glm::dvec3& position,
                         const glm::dvec3& momentum, uint64_t particleId) {
    m_hits.push_back({time, position, momentum, particleId});
}

} // namespace pas::accelerator
