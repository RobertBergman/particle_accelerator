#pragma once

#include "accelerator/Component.hpp"
#include "physics/EMField.hpp"
#include <vector>
#include <memory>
#include <optional>

namespace pas::accelerator {

/**
 * @brief Type of accelerator lattice.
 */
enum class LatticeType {
    Linear,    // Single-pass (linac)
    Circular   // Periodic boundary (synchrotron/storage ring)
};

/**
 * @brief FODO cell parameters for standard lattice construction.
 */
struct FODOCellParams {
    double cellLength = 10.0;      // Total cell length (m)
    double quadLength = 0.5;       // Quadrupole length (m)
    double quadGradient = 50.0;    // Quadrupole gradient magnitude (T/m)
    double driftLength = 0.0;      // Will be computed if 0
    double aperture = 0.05;        // Aperture radius (m)
};

/**
 * @brief Container for accelerator components (the Lattice).
 *
 * Manages the arrangement of beamline elements and provides
 * field integration with the physics engine.
 */
class Accelerator {
public:
    Accelerator();

    /**
     * @brief Set the lattice type.
     */
    void setLatticeType(LatticeType type) { m_latticeType = type; }
    LatticeType getLatticeType() const { return m_latticeType; }

    /**
     * @brief Add a component to the end of the beamline.
     */
    void addComponent(std::shared_ptr<Component> component);

    /**
     * @brief Insert a component at a specific index.
     */
    void insertComponent(size_t index, std::shared_ptr<Component> component);

    /**
     * @brief Remove a component by index.
     */
    void removeComponent(size_t index);

    /**
     * @brief Remove a component by name.
     */
    void removeComponent(const std::string& name);

    /**
     * @brief Clear all components.
     */
    void clear();

    /**
     * @brief Get the number of components.
     */
    size_t getComponentCount() const { return m_components.size(); }

    /**
     * @brief Get a component by index.
     */
    std::shared_ptr<Component> getComponent(size_t index) const;

    /**
     * @brief Get a component by name.
     */
    std::shared_ptr<Component> getComponent(const std::string& name) const;

    /**
     * @brief Get all components.
     */
    const std::vector<std::shared_ptr<Component>>& getComponents() const {
        return m_components;
    }

    /**
     * @brief Find the component at a given s-position.
     */
    std::shared_ptr<Component> getComponentAtS(double s) const;

    // Lattice construction helpers

    /**
     * @brief Build a standard FODO cell.
     *
     * A FODO cell consists of: QF - Drift - QD - Drift
     * where QF is focusing and QD is defocusing.
     *
     * @param params FODO cell parameters.
     * @param cellName Base name for the cell components.
     */
    void buildFODOCell(const FODOCellParams& params,
                       const std::string& cellName = "FODO");

    /**
     * @brief Build multiple FODO cells.
     */
    void buildFODOLattice(const FODOCellParams& params, size_t numCells);

    /**
     * @brief Add a drift section.
     */
    void addDrift(double length, const std::string& name = "");

    /**
     * @brief Compute the lattice (calculate s-positions).
     *
     * Should be called after adding all components and before simulation.
     */
    void computeLattice();

    /**
     * @brief Close the ring for circular machines.
     *
     * Connects the end back to the beginning.
     */
    void closeRing();

    // Lattice properties

    /**
     * @brief Get the total length of the beamline.
     */
    double getTotalLength() const { return m_totalLength; }

    /**
     * @brief Get the circumference (for circular machines).
     */
    double getCircumference() const { return m_totalLength; }

    /**
     * @brief Check if the lattice is closed (circular).
     */
    bool isClosed() const { return m_latticeType == LatticeType::Circular; }

    // Field integration

    /**
     * @brief Populate the field manager with all component fields.
     *
     * Call this to register all fields with the physics engine.
     */
    void populateFieldManager(physics::EMFieldManager& manager) const;

    /**
     * @brief Get all dipoles in the lattice.
     */
    std::vector<std::shared_ptr<Dipole>> getDipoles() const;

    /**
     * @brief Get all quadrupoles in the lattice.
     */
    std::vector<std::shared_ptr<Quadrupole>> getQuadrupoles() const;

    /**
     * @brief Get all RF cavities in the lattice.
     */
    std::vector<std::shared_ptr<RFCavity>> getRFCavities() const;

    // Statistics

    /**
     * @brief Get the number of dipoles.
     */
    size_t getDipoleCount() const;

    /**
     * @brief Get the number of quadrupoles.
     */
    size_t getQuadrupoleCount() const;

    /**
     * @brief Calculate the total bending angle.
     */
    double getTotalBendingAngle(double momentum) const;

private:
    void updateSPositions();

    std::vector<std::shared_ptr<Component>> m_components;
    LatticeType m_latticeType = LatticeType::Linear;
    double m_totalLength = 0.0;
    size_t m_driftCounter = 0;
};

} // namespace pas::accelerator
