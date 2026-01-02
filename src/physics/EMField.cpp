#include "physics/EMField.hpp"
#include "physics/Constants.hpp"
#include <algorithm>
#include <cmath>

namespace pas::physics {

// EMFieldManager implementation

void EMFieldManager::addSource(std::shared_ptr<FieldSource> source) {
    if (source) {
        m_sources.push_back(std::move(source));
    }
}

void EMFieldManager::removeSource(const std::shared_ptr<FieldSource>& source) {
    auto it = std::find(m_sources.begin(), m_sources.end(), source);
    if (it != m_sources.end()) {
        m_sources.erase(it);
    }
}

void EMFieldManager::clear() {
    m_sources.clear();
}

FieldValue EMFieldManager::evaluate(const glm::dvec3& position, double time) const {
    FieldValue total;
    for (const auto& source : m_sources) {
        if (source && source->isEnabled() && source->isInside(position)) {
            total += source->evaluate(position, time);
        }
    }
    return total;
}

// UniformBField implementation

UniformBField::UniformBField(const glm::dvec3& field, const BoundingBox& bounds)
    : m_field(field)
    , m_bounds(bounds) {
}

FieldValue UniformBField::evaluate(const glm::dvec3& position, double /*time*/) const {
    if (!m_bounds.isInfinite() && !m_bounds.contains(position)) {
        return FieldValue();
    }
    return FieldValue(glm::dvec3(0.0), m_field);
}

// QuadrupoleField implementation

QuadrupoleField::QuadrupoleField(double gradient,
                                 const glm::dvec3& center,
                                 double length,
                                 double aperture)
    : m_gradient(gradient)
    , m_center(center)
    , m_length(length)
    , m_aperture(aperture) {
    // Calculate bounding box
    double halfLength = length / 2.0;
    m_bounds = BoundingBox(
        glm::dvec3(center.x - aperture, center.y - aperture, center.z - halfLength),
        glm::dvec3(center.x + aperture, center.y + aperture, center.z + halfLength)
    );
}

FieldValue QuadrupoleField::evaluate(const glm::dvec3& position, double /*time*/) const {
    // Check if inside aperture and length
    if (!m_bounds.contains(position)) {
        return FieldValue();
    }

    // Local coordinates relative to center
    double x = position.x - m_center.x;
    double y = position.y - m_center.y;

    // Check radial aperture
    double r = std::sqrt(x * x + y * y);
    if (r > m_aperture) {
        return FieldValue();
    }

    // Quadrupole field: Bx = G * y, By = G * x
    glm::dvec3 B(m_gradient * y, m_gradient * x, 0.0);
    return FieldValue(glm::dvec3(0.0), B);
}

// RFField implementation

RFField::RFField(double voltage,
                 double frequency,
                 double phase,
                 const glm::dvec3& center,
                 double length,
                 double aperture)
    : m_voltage(voltage)
    , m_frequency(frequency)
    , m_omega(2.0 * constants::pi * frequency)
    , m_phase(phase)
    , m_center(center)
    , m_length(length)
    , m_aperture(aperture) {
    // Calculate bounding box
    double halfLength = length / 2.0;
    m_bounds = BoundingBox(
        glm::dvec3(center.x - aperture, center.y - aperture, center.z - halfLength),
        glm::dvec3(center.x + aperture, center.y + aperture, center.z + halfLength)
    );
}

FieldValue RFField::evaluate(const glm::dvec3& position, double time) const {
    if (!m_bounds.contains(position)) {
        return FieldValue();
    }

    // Check radial aperture
    double x = position.x - m_center.x;
    double y = position.y - m_center.y;
    double r = std::sqrt(x * x + y * y);
    if (r > m_aperture) {
        return FieldValue();
    }

    // E_z = (V/L) * cos(omega * t + phi)
    double Ez = (m_voltage / m_length) * std::cos(m_omega * time + m_phase);
    return FieldValue(glm::dvec3(0.0, 0.0, Ez), glm::dvec3(0.0));
}

void RFField::setFrequency(double frequency) {
    m_frequency = frequency;
    m_omega = 2.0 * constants::pi * frequency;
}

} // namespace pas::physics
