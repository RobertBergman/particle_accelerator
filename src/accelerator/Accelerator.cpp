#include "accelerator/Accelerator.hpp"
#include <algorithm>
#include <stdexcept>

namespace pas::accelerator {

Accelerator::Accelerator() = default;

void Accelerator::addComponent(std::shared_ptr<Component> component) {
    if (component) {
        m_components.push_back(std::move(component));
    }
}

void Accelerator::insertComponent(size_t index, std::shared_ptr<Component> component) {
    if (component && index <= m_components.size()) {
        m_components.insert(m_components.begin() + static_cast<ptrdiff_t>(index),
                            std::move(component));
    }
}

void Accelerator::removeComponent(size_t index) {
    if (index < m_components.size()) {
        m_components.erase(m_components.begin() + static_cast<ptrdiff_t>(index));
    }
}

void Accelerator::removeComponent(const std::string& name) {
    m_components.erase(
        std::remove_if(m_components.begin(), m_components.end(),
                       [&name](const auto& c) { return c->getName() == name; }),
        m_components.end()
    );
}

void Accelerator::clear() {
    m_components.clear();
    m_totalLength = 0.0;
    m_driftCounter = 0;
}

std::shared_ptr<Component> Accelerator::getComponent(size_t index) const {
    if (index < m_components.size()) {
        return m_components[index];
    }
    return nullptr;
}

std::shared_ptr<Component> Accelerator::getComponent(const std::string& name) const {
    auto it = std::find_if(m_components.begin(), m_components.end(),
                           [&name](const auto& c) { return c->getName() == name; });
    if (it != m_components.end()) {
        return *it;
    }
    return nullptr;
}

std::shared_ptr<Component> Accelerator::getComponentAtS(double s) const {
    // Handle circular case
    if (m_latticeType == LatticeType::Circular && m_totalLength > 0) {
        s = std::fmod(s, m_totalLength);
        if (s < 0) s += m_totalLength;
    }

    for (const auto& component : m_components) {
        if (component->containsS(s)) {
            return component;
        }
    }
    return nullptr;
}

void Accelerator::buildFODOCell(const FODOCellParams& params,
                                 const std::string& cellName) {
    double quadLength = params.quadLength;
    double cellLength = params.cellLength;

    // Calculate drift length if not specified
    double driftLength = params.driftLength;
    if (driftLength <= 0) {
        // Cell = QF/2 + Drift + QD + Drift + QF/2
        // Using thin lens approximation: effective is half-quad at each end
        driftLength = (cellLength - 2.0 * quadLength) / 2.0;
    }

    Aperture aperture;
    aperture.radiusX = params.aperture;
    aperture.radiusY = params.aperture;

    // QF (Focusing quadrupole)
    auto qf = std::make_shared<Quadrupole>(
        cellName + "_QF",
        quadLength,
        params.quadGradient,  // Positive = focusing in x
        aperture
    );
    addComponent(qf);

    // Drift 1
    addDrift(driftLength, cellName + "_D1");

    // QD (Defocusing quadrupole)
    auto qd = std::make_shared<Quadrupole>(
        cellName + "_QD",
        quadLength,
        -params.quadGradient,  // Negative = defocusing in x
        aperture
    );
    addComponent(qd);

    // Drift 2
    addDrift(driftLength, cellName + "_D2");
}

void Accelerator::buildFODOLattice(const FODOCellParams& params, size_t numCells) {
    for (size_t i = 0; i < numCells; ++i) {
        std::string cellName = "FODO_" + std::to_string(i + 1);
        buildFODOCell(params, cellName);
    }
}

void Accelerator::addDrift(double length, const std::string& name) {
    std::string driftName = name.empty()
        ? "Drift_" + std::to_string(++m_driftCounter)
        : name;

    auto drift = std::make_shared<BeamPipe>(driftName, length);
    addComponent(drift);
}

void Accelerator::computeLattice() {
    updateSPositions();
}

void Accelerator::closeRing() {
    m_latticeType = LatticeType::Circular;
    updateSPositions();
}

void Accelerator::updateSPositions() {
    double s = 0.0;
    for (auto& component : m_components) {
        component->setSPosition(s);
        s += component->getLength();
    }
    m_totalLength = s;
}

void Accelerator::populateFieldManager(physics::EMFieldManager& manager) const {
    for (const auto& component : m_components) {
        auto field = component->getFieldSource();
        if (field) {
            manager.addSource(field);
        }
    }
}

std::vector<std::shared_ptr<Dipole>> Accelerator::getDipoles() const {
    std::vector<std::shared_ptr<Dipole>> dipoles;
    for (const auto& component : m_components) {
        if (component->getType() == ComponentType::Dipole) {
            dipoles.push_back(std::dynamic_pointer_cast<Dipole>(component));
        }
    }
    return dipoles;
}

std::vector<std::shared_ptr<Quadrupole>> Accelerator::getQuadrupoles() const {
    std::vector<std::shared_ptr<Quadrupole>> quads;
    for (const auto& component : m_components) {
        if (component->getType() == ComponentType::Quadrupole) {
            quads.push_back(std::dynamic_pointer_cast<Quadrupole>(component));
        }
    }
    return quads;
}

std::vector<std::shared_ptr<RFCavity>> Accelerator::getRFCavities() const {
    std::vector<std::shared_ptr<RFCavity>> cavities;
    for (const auto& component : m_components) {
        if (component->getType() == ComponentType::RFCavity) {
            cavities.push_back(std::dynamic_pointer_cast<RFCavity>(component));
        }
    }
    return cavities;
}

size_t Accelerator::getDipoleCount() const {
    return std::count_if(m_components.begin(), m_components.end(),
                         [](const auto& c) { return c->getType() == ComponentType::Dipole; });
}

size_t Accelerator::getQuadrupoleCount() const {
    return std::count_if(m_components.begin(), m_components.end(),
                         [](const auto& c) { return c->getType() == ComponentType::Quadrupole; });
}

double Accelerator::getTotalBendingAngle(double momentum) const {
    double totalAngle = 0.0;
    for (const auto& dipole : getDipoles()) {
        totalAngle += dipole->getBendingAngle(momentum);
    }
    return totalAngle;
}

} // namespace pas::accelerator
