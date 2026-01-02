#pragma once

#include "physics/PhysicsEngine.hpp"
#include "accelerator/Accelerator.hpp"
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace pas::config {

/**
 * @brief Configuration manager for saving/loading simulation settings.
 */
class Config {
public:
    /**
     * @brief Simulation configuration data.
     */
    struct SimulationConfig {
        double timeStep = 1e-11;
        double timeScale = 1e6;
        int integratorType = 2;  // Boris
        size_t particleCount = 1000;
        double beamEnergy = 1e9;  // eV
    };

    /**
     * @brief Window configuration data.
     */
    struct WindowConfig {
        int width = 1600;
        int height = 900;
        bool vsync = true;
        bool fullscreen = false;
    };

    /**
     * @brief Rendering configuration data.
     */
    struct RenderConfig {
        bool wireframe = false;
        bool showGrid = true;
        bool showAxes = true;
        float particleSize = 2.0f;
        int colorScheme = 0;  // ByEnergy
    };

    Config();

    /**
     * @brief Load configuration from JSON file.
     */
    bool load(const std::string& filepath);

    /**
     * @brief Save configuration to JSON file.
     */
    bool save(const std::string& filepath) const;

    /**
     * @brief Load default configuration.
     */
    void loadDefaults();

    // Configuration accessors
    SimulationConfig& simulation() { return m_simulation; }
    const SimulationConfig& simulation() const { return m_simulation; }

    WindowConfig& window() { return m_window; }
    const WindowConfig& window() const { return m_window; }

    RenderConfig& render() { return m_render; }
    const RenderConfig& render() const { return m_render; }

    /**
     * @brief Apply configuration to physics engine.
     */
    void applyToEngine(physics::PhysicsEngine& engine) const;

    /**
     * @brief Load accelerator lattice from JSON file.
     */
    static std::shared_ptr<accelerator::Accelerator>
    loadAccelerator(const std::string& filepath);

    /**
     * @brief Save accelerator lattice to JSON file.
     */
    static bool saveAccelerator(const accelerator::Accelerator& accelerator,
                                const std::string& filepath);

private:
    SimulationConfig m_simulation;
    WindowConfig m_window;
    RenderConfig m_render;
};

// JSON serialization for nlohmann/json
void to_json(nlohmann::json& j, const Config::SimulationConfig& c);
void from_json(const nlohmann::json& j, Config::SimulationConfig& c);
void to_json(nlohmann::json& j, const Config::WindowConfig& c);
void from_json(const nlohmann::json& j, Config::WindowConfig& c);
void to_json(nlohmann::json& j, const Config::RenderConfig& c);
void from_json(const nlohmann::json& j, Config::RenderConfig& c);

} // namespace pas::config
