#include "config/Config.hpp"
#include "utils/Logger.hpp"
#include "physics/Constants.hpp"

#include <fstream>

namespace pas::config {

// JSON serialization implementations
void to_json(nlohmann::json& j, const Config::SimulationConfig& c) {
    j = nlohmann::json{
        {"timeStep", c.timeStep},
        {"timeScale", c.timeScale},
        {"integratorType", c.integratorType},
        {"particleCount", c.particleCount},
        {"beamEnergy", c.beamEnergy}
    };
}

void from_json(const nlohmann::json& j, Config::SimulationConfig& c) {
    if (j.contains("timeStep")) j.at("timeStep").get_to(c.timeStep);
    if (j.contains("timeScale")) j.at("timeScale").get_to(c.timeScale);
    if (j.contains("integratorType")) j.at("integratorType").get_to(c.integratorType);
    if (j.contains("particleCount")) j.at("particleCount").get_to(c.particleCount);
    if (j.contains("beamEnergy")) j.at("beamEnergy").get_to(c.beamEnergy);
}

void to_json(nlohmann::json& j, const Config::WindowConfig& c) {
    j = nlohmann::json{
        {"width", c.width},
        {"height", c.height},
        {"vsync", c.vsync},
        {"fullscreen", c.fullscreen}
    };
}

void from_json(const nlohmann::json& j, Config::WindowConfig& c) {
    if (j.contains("width")) j.at("width").get_to(c.width);
    if (j.contains("height")) j.at("height").get_to(c.height);
    if (j.contains("vsync")) j.at("vsync").get_to(c.vsync);
    if (j.contains("fullscreen")) j.at("fullscreen").get_to(c.fullscreen);
}

void to_json(nlohmann::json& j, const Config::RenderConfig& c) {
    j = nlohmann::json{
        {"wireframe", c.wireframe},
        {"showGrid", c.showGrid},
        {"showAxes", c.showAxes},
        {"particleSize", c.particleSize},
        {"colorScheme", c.colorScheme}
    };
}

void from_json(const nlohmann::json& j, Config::RenderConfig& c) {
    if (j.contains("wireframe")) j.at("wireframe").get_to(c.wireframe);
    if (j.contains("showGrid")) j.at("showGrid").get_to(c.showGrid);
    if (j.contains("showAxes")) j.at("showAxes").get_to(c.showAxes);
    if (j.contains("particleSize")) j.at("particleSize").get_to(c.particleSize);
    if (j.contains("colorScheme")) j.at("colorScheme").get_to(c.colorScheme);
}

Config::Config() {
    loadDefaults();
}

void Config::loadDefaults() {
    m_simulation = SimulationConfig{};
    m_window = WindowConfig{};
    m_render = RenderConfig{};
}

bool Config::load(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            PAS_WARN("Config: Could not open file: {}", filepath);
            return false;
        }

        nlohmann::json j;
        file >> j;

        if (j.contains("simulation")) {
            m_simulation = j["simulation"].get<SimulationConfig>();
        }
        if (j.contains("window")) {
            m_window = j["window"].get<WindowConfig>();
        }
        if (j.contains("render")) {
            m_render = j["render"].get<RenderConfig>();
        }

        PAS_INFO("Config: Loaded configuration from {}", filepath);
        return true;
    } catch (const std::exception& e) {
        PAS_ERROR("Config: Error loading {}: {}", filepath, e.what());
        return false;
    }
}

bool Config::save(const std::string& filepath) const {
    try {
        nlohmann::json j;
        j["simulation"] = m_simulation;
        j["window"] = m_window;
        j["render"] = m_render;

        std::ofstream file(filepath);
        if (!file.is_open()) {
            PAS_ERROR("Config: Could not create file: {}", filepath);
            return false;
        }

        file << j.dump(4);
        PAS_INFO("Config: Saved configuration to {}", filepath);
        return true;
    } catch (const std::exception& e) {
        PAS_ERROR("Config: Error saving {}: {}", filepath, e.what());
        return false;
    }
}

void Config::applyToEngine(physics::PhysicsEngine& engine) const {
    engine.setTimeStep(m_simulation.timeStep);
    engine.setTimeScale(m_simulation.timeScale);
    engine.setIntegrator(static_cast<physics::IntegratorFactory::Type>(m_simulation.integratorType));
}

std::shared_ptr<accelerator::Accelerator>
Config::loadAccelerator(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            PAS_WARN("Config: Could not open accelerator file: {}", filepath);
            return nullptr;
        }

        nlohmann::json j;
        file >> j;

        auto acc = std::make_shared<accelerator::Accelerator>();

        // Parse lattice type
        if (j.contains("latticeType")) {
            std::string type = j["latticeType"];
            if (type == "circular") {
                acc->setLatticeType(accelerator::LatticeType::Circular);
            }
        }

        // Parse components
        if (j.contains("components") && j["components"].is_array()) {
            for (const auto& comp : j["components"]) {
                std::string type = comp.value("type", "");
                std::string name = comp.value("name", "unnamed");
                double length = comp.value("length", 1.0);
                double aperture = comp.value("aperture", 0.05);

                accelerator::Aperture ap;
                ap.radiusX = aperture;
                ap.radiusY = aperture;

                if (type == "drift" || type == "beampipe") {
                    auto pipe = std::make_shared<accelerator::BeamPipe>(name, length, ap);
                    acc->addComponent(pipe);
                } else if (type == "dipole") {
                    double field = comp.value("field", 1.0);
                    auto dipole = std::make_shared<accelerator::Dipole>(name, length, field, ap);
                    acc->addComponent(dipole);
                } else if (type == "quadrupole") {
                    double gradient = comp.value("gradient", 10.0);
                    auto quad = std::make_shared<accelerator::Quadrupole>(name, length, gradient, ap);
                    acc->addComponent(quad);
                } else if (type == "rfcavity") {
                    double voltage = comp.value("voltage", 1e6);
                    double frequency = comp.value("frequency", 500e6);
                    double phase = comp.value("phase", 0.0);
                    auto rf = std::make_shared<accelerator::RFCavity>(
                        name, length, voltage, frequency, phase, ap);
                    acc->addComponent(rf);
                }
            }
        }

        acc->computeLattice();

        PAS_INFO("Config: Loaded accelerator from {} with {} components",
                 filepath, acc->getComponentCount());
        return acc;
    } catch (const std::exception& e) {
        PAS_ERROR("Config: Error loading accelerator from {}: {}", filepath, e.what());
        return nullptr;
    }
}

bool Config::saveAccelerator(const accelerator::Accelerator& accelerator,
                              const std::string& filepath) {
    try {
        nlohmann::json j;

        j["latticeType"] = accelerator.getLatticeType() == accelerator::LatticeType::Circular
                          ? "circular" : "linear";
        j["totalLength"] = accelerator.getTotalLength();

        nlohmann::json components = nlohmann::json::array();
        for (const auto& comp : accelerator.getComponents()) {
            nlohmann::json c;
            c["name"] = comp->getName();
            c["length"] = comp->getLength();
            c["aperture"] = comp->getAperture().radiusX;
            c["sPosition"] = comp->getSPosition();

            switch (comp->getType()) {
                case accelerator::ComponentType::BeamPipe:
                    c["type"] = "beampipe";
                    break;
                case accelerator::ComponentType::Dipole:
                    c["type"] = "dipole";
                    if (auto d = std::dynamic_pointer_cast<accelerator::Dipole>(comp)) {
                        c["field"] = d->getField();
                    }
                    break;
                case accelerator::ComponentType::Quadrupole:
                    c["type"] = "quadrupole";
                    if (auto q = std::dynamic_pointer_cast<accelerator::Quadrupole>(comp)) {
                        c["gradient"] = q->getGradient();
                    }
                    break;
                case accelerator::ComponentType::RFCavity:
                    c["type"] = "rfcavity";
                    if (auto rf = std::dynamic_pointer_cast<accelerator::RFCavity>(comp)) {
                        c["voltage"] = rf->getVoltage();
                        c["frequency"] = rf->getFrequency();
                        c["phase"] = rf->getPhase();
                    }
                    break;
                default:
                    c["type"] = "unknown";
                    break;
            }

            components.push_back(c);
        }
        j["components"] = components;

        std::ofstream file(filepath);
        if (!file.is_open()) {
            PAS_ERROR("Config: Could not create file: {}", filepath);
            return false;
        }

        file << j.dump(4);
        PAS_INFO("Config: Saved accelerator to {}", filepath);
        return true;
    } catch (const std::exception& e) {
        PAS_ERROR("Config: Error saving accelerator to {}: {}", filepath, e.what());
        return false;
    }
}

} // namespace pas::config
