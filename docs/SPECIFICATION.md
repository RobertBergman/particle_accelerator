# Particle Accelerator Simulation - Technical Specification

## Version 1.0.0

---

## Table of Contents

1. [Overview](#1-overview)
2. [System Architecture](#2-system-architecture)
3. [Core Components](#3-core-components)
4. [Physics Engine](#4-physics-engine)
5. [Rendering System](#5-rendering-system)
6. [Particle System](#6-particle-system)
7. [Accelerator Components](#7-accelerator-components)
8. [User Interface](#8-user-interface)
9. [Data Management](#9-data-management)
10. [Configuration System](#10-configuration-system)
11. [Build System](#11-build-system)
12. [Testing Strategy](#12-testing-strategy)

---

## 1. Overview

### 1.1 Purpose

A real-time, interactive 3D simulation of a particle accelerator using C++ and OpenGL. The simulation models charged particle dynamics through electromagnetic fields, visualizes particle beams, and allows users to interact with accelerator parameters.

### 1.2 Key Features

- Real-time particle physics simulation with relativistic corrections
- 3D visualization using modern OpenGL (4.5+)
- Interactive camera controls and parameter adjustment
- Support for circular (synchrotron) and linear (linac) accelerator configurations
- Beam diagnostics and statistics display
- Configuration save/load functionality
- Performance-optimized for 100,000+ particles

### 1.3 Target Platform

- **OS**: Windows 10/11, Linux
- **Graphics**: OpenGL 4.5+ compatible GPU
- **Memory**: 8GB+ RAM recommended
- **CPU**: Multi-core processor (simulation parallelized via OpenMP/threads)

### 1.4 Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| GLFW | 3.3+ | Window/input management |
| GLAD | 0.1.36+ | OpenGL function loading |
| GLM | 0.9.9+ | Mathematics library |
| Dear ImGui | 1.89+ | User interface |
| stb_image | 2.28+ | Texture loading |
| nlohmann/json | 3.11+ | Configuration files |
| spdlog | 1.12+ | Logging |
| GoogleTest | 1.13+ | Unit tests (dev/test) |
| Google Benchmark | 1.8+ | Performance benchmarks (dev/test) |

### 1.5 Units, Coordinate System, and Conventions

- Coordinate system is right-handed: +X right, +Y up, +Z forward (linac beamline direction).
- For circular machines, the local beam frame uses (x, y, s) with s along the reference orbit, x radial outward, y vertical.
- Units are SI unless stated: meters, seconds, kilograms, Coulombs, radians; E in V/m and B in Tesla.
- Energies are represented internally in Joules; configuration files specify energies in eV and are converted on load.
- Momentum uses kg*m/s; `beamMomentum.delta` is relative momentum deviation (unitless).

### 1.6 Determinism and Reproducibility

- Runs are reproducible given the same config, random seed, and platform.
- Beam generation and any stochastic effects use a deterministic RNG seeded via config; if omitted, a time-based seed is used and recorded in logs.
- Parallel particle updates may reorder floating-point reductions; aggregate statistics should remain stable within tolerance.

---

## 2. System Architecture

### 2.1 High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Layer                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │   Main      │  │   Config    │  │    State Manager        │  │
│  │   Loop      │  │   Manager   │  │                         │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                        Simulation Layer                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  Physics    │  │  Particle   │  │    Accelerator          │  │
│  │  Engine     │  │  System     │  │    Components           │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                        Rendering Layer                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  Renderer   │  │  Shader     │  │    Camera               │  │
│  │             │  │  Manager    │  │    System               │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                        Platform Layer                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  Window     │  │  Input      │  │    Resource             │  │
│  │  Manager    │  │  Handler    │  │    Manager              │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Directory Structure

```
particle_accelerator/
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Application.hpp/cpp
│   │   ├── Window.hpp/cpp
│   │   ├── Input.hpp/cpp
│   │   └── Timer.hpp/cpp
│   ├── physics/
│   │   ├── PhysicsEngine.hpp/cpp
│   │   ├── Particle.hpp/cpp
│   │   ├── ParticleSystem.hpp/cpp
│   │   ├── EMField.hpp/cpp
│   │   ├── Integrator.hpp/cpp
│   │   └── Constants.hpp
│   ├── accelerator/
│   │   ├── Accelerator.hpp/cpp
│   │   ├── BeamPipe.hpp/cpp
│   │   ├── Dipole.hpp/cpp
│   │   ├── Quadrupole.hpp/cpp
│   │   ├── RFCavity.hpp/cpp
│   │   ├── Detector.hpp/cpp
│   │   └── ComponentFactory.hpp/cpp
│   ├── rendering/
│   │   ├── Renderer.hpp/cpp
│   │   ├── Camera.hpp/cpp
│   │   ├── Shader.hpp/cpp
│   │   ├── ShaderManager.hpp/cpp
│   │   ├── Mesh.hpp/cpp
│   │   ├── MeshFactory.hpp/cpp
│   │   ├── ParticleRenderer.hpp/cpp
│   │   ├── AcceleratorRenderer.hpp/cpp
│   │   ├── FieldVisualizer.hpp/cpp
│   │   └── PostProcessor.hpp/cpp
│   ├── ui/
│   │   ├── UIManager.hpp/cpp
│   │   ├── ControlPanel.hpp/cpp
│   │   ├── DiagnosticsPanel.hpp/cpp
│   │   ├── BeamStatsPanel.hpp/cpp
│   │   └── SettingsPanel.hpp/cpp
│   ├── data/
│   │   ├── ConfigManager.hpp/cpp
│   │   ├── DataLogger.hpp/cpp
│   │   └── ExportManager.hpp/cpp
│   └── utils/
│       ├── Logger.hpp/cpp
│       ├── MathUtils.hpp
│       ├── Random.hpp/cpp
│       └── ThreadPool.hpp/cpp
├── shaders/
│   ├── particle.vert
│   ├── particle.frag
│   ├── particle.geom
│   ├── accelerator.vert
│   ├── accelerator.frag
│   ├── field.vert
│   ├── field.frag
│   ├── postprocess.vert
│   ├── postprocess.frag
│   └── skybox.vert/frag
├── assets/
│   ├── textures/
│   ├── models/
│   └── fonts/
├── config/
│   ├── default.json
│   └── accelerators/
│       ├── synchrotron.json
│       └── linac.json
├── tests/
│   ├── physics/
│   ├── rendering/
│   └── integration/
├── docs/
│   └── SPECIFICATION.md
├── CMakeLists.txt
└── README.md
```

---

## 3. Core Components

### 3.1 Application Class

```cpp
// src/core/Application.hpp

#pragma once

#include <memory>
#include <string>

namespace pas { // Particle Accelerator Simulation

class Window;
class Renderer;
class PhysicsEngine;
class UIManager;
class ConfigManager;

enum class SimulationState {
    Stopped,
    Running,
    Paused,
    Stepping
};

class Application {
public:
    struct Config {
        std::string windowTitle = "Particle Accelerator Simulation";
        int windowWidth = 1920;
        int windowHeight = 1080;
        bool fullscreen = false;
        bool vsync = true;
        int targetFPS = 60;
        std::string configPath = "config/default.json";
    };

    Application(const Config& config);
    ~Application();

    // Non-copyable, non-movable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();
    void stop();

    // State control
    void startSimulation();
    void pauseSimulation();
    void resetSimulation();
    void stepSimulation();

    // Accessors
    SimulationState getState() const;
    double getSimulationTime() const;
    double getDeltaTime() const;

    // Singleton access (optional)
    static Application& getInstance();

private:
    void initialize();
    void shutdown();
    void mainLoop();
    void update(double deltaTime);
    void render();
    void processInput();

    Config m_config;
    SimulationState m_state = SimulationState::Stopped;
    bool m_running = false;
    double m_simulationTime = 0.0;
    double m_deltaTime = 0.0;
    double m_timeScale = 1.0;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<PhysicsEngine> m_physics;
    std::unique_ptr<UIManager> m_ui;
    std::unique_ptr<ConfigManager> m_configManager;
};

} // namespace pas
```

### 3.2 Window Class

```cpp
// src/core/Window.hpp

#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

namespace pas {

class Window {
public:
    struct Properties {
        std::string title;
        int width;
        int height;
        bool fullscreen;
        bool vsync;
        int samples = 4; // MSAA samples
    };

    using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(int, int, int, int)>;
    using MouseCallback = std::function<void(double, double)>;
    using ScrollCallback = std::function<void(double, double)>;
    using MouseButtonCallback = std::function<void(int, int, int)>;

    explicit Window(const Properties& props);
    ~Window();

    void pollEvents();
    void swapBuffers();
    bool shouldClose() const;
    void close();

    // Properties
    int getWidth() const;
    int getHeight() const;
    float getAspectRatio() const;
    GLFWwindow* getNativeHandle() const;

    // Callbacks
    void setResizeCallback(ResizeCallback callback);
    void setKeyCallback(KeyCallback callback);
    void setMouseCallback(MouseCallback callback);
    void setScrollCallback(ScrollCallback callback);
    void setMouseButtonCallback(MouseButtonCallback callback);

    // Cursor
    void setCursorMode(int mode);
    void getCursorPos(double& x, double& y) const;

private:
    GLFWwindow* m_window = nullptr;
    Properties m_props;

    ResizeCallback m_resizeCallback;
    KeyCallback m_keyCallback;
    MouseCallback m_mouseCallback;
    ScrollCallback m_scrollCallback;
    MouseButtonCallback m_mouseButtonCallback;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

} // namespace pas
```

### 3.3 Input Handler

```cpp
// src/core/Input.hpp

#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

namespace pas {

class Window;

class Input {
public:
    static void initialize(Window* window);
    static void update();

    // Keyboard
    static bool isKeyPressed(int keycode);
    static bool isKeyJustPressed(int keycode);
    static bool isKeyJustReleased(int keycode);

    // Mouse
    static bool isMouseButtonPressed(int button);
    static bool isMouseButtonJustPressed(int button);
    static glm::vec2 getMousePosition();
    static glm::vec2 getMouseDelta();
    static float getScrollOffset();

private:
    static Window* s_window;
    static std::unordered_map<int, bool> s_keyState;
    static std::unordered_map<int, bool> s_prevKeyState;
    static std::unordered_map<int, bool> s_mouseState;
    static std::unordered_map<int, bool> s_prevMouseState;
    static glm::vec2 s_mousePos;
    static glm::vec2 s_prevMousePos;
    static float s_scrollOffset;
};

} // namespace pas
```

### 3.4 Timer Class

```cpp
// src/core/Timer.hpp

#pragma once

#include <chrono>

namespace pas {

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;

    Timer();

    void reset();
    double elapsed() const;       // Seconds since last reset
    double elapsedMillis() const; // Milliseconds since last reset

    // Frame timing
    void tick();
    double getDeltaTime() const;
    double getFPS() const;

private:
    TimePoint m_start;
    TimePoint m_lastTick;
    double m_deltaTime = 0.0;
    double m_fps = 0.0;

    // FPS smoothing
    static constexpr int FPS_SAMPLES = 60;
    double m_frameTimes[FPS_SAMPLES] = {0.0};
    int m_frameIndex = 0;
};

} // namespace pas
```

---

## 4. Physics Engine

### 4.1 Physical Constants

```cpp
// src/physics/Constants.hpp

#pragma once

namespace pas::physics {

// Fundamental constants (SI units)
namespace constants {
    constexpr double c = 299792458.0;              // Speed of light (m/s)
    constexpr double e = 1.602176634e-19;          // Elementary charge (C)
    constexpr double m_e = 9.1093837015e-31;       // Electron mass (kg)
    constexpr double m_p = 1.67262192369e-27;      // Proton mass (kg)
    constexpr double epsilon_0 = 8.8541878128e-12; // Vacuum permittivity (F/m)
    constexpr double mu_0 = 1.25663706212e-6;      // Vacuum permeability (H/m)
    constexpr double k_B = 1.380649e-23;           // Boltzmann constant (J/K)
    constexpr double eV = 1.602176634e-19;         // Electron volt (J)
    constexpr double MeV = 1.602176634e-13;        // Mega electron volt (J)
    constexpr double GeV = 1.602176634e-10;        // Giga electron volt (J)
}

// Particle types
enum class ParticleType {
    Electron,
    Positron,
    Proton,
    Antiproton,
    Custom
};

// Get particle properties
struct ParticleProperties {
    double mass;      // kg
    double charge;    // C
    std::string name;
};

ParticleProperties getParticleProperties(ParticleType type);

} // namespace pas::physics
```

### 4.2 Particle Class

```cpp
// src/physics/Particle.hpp

#pragma once

#include <glm/glm.hpp>
#include "Constants.hpp"

namespace pas::physics {

struct Particle {
    // State
    glm::dvec3 position{0.0};    // meters
    glm::dvec3 velocity{0.0};    // m/s
    glm::dvec3 momentum{0.0};    // kg⋅m/s (relativistic)

    // Properties
    double mass;                  // kg
    double charge;                // Coulombs
    double restEnergy;            // Joules (m*c^2)

    // Derived quantities (updated each step)
    double gamma = 1.0;           // Lorentz factor
    double beta = 0.0;            // v/c
    double kineticEnergy = 0.0;   // Joules
    double totalEnergy = 0.0;     // Joules

    // Tracking
    uint32_t id = 0;
    bool active = true;
    double age = 0.0;             // seconds since injection
    int turns = 0;                // for circular accelerators

    // Beam coordinates (relative to reference orbit)
    glm::dvec3 beamPosition{0.0}; // (x, y, s) - transverse + longitudinal
    glm::dvec3 beamMomentum{0.0}; // (px, py, delta) - angles + momentum deviation

    // Constructor
    Particle(ParticleType type = ParticleType::Proton);
    Particle(double mass, double charge);

    // Methods
    void updateDerivedQuantities();
    void setKineticEnergy(double energy);
    void setMomentum(const glm::dvec3& p);
    void setVelocity(const glm::dvec3& v);

    // Relativistic calculations
    double getSpeed() const;
    glm::dvec3 getVelocity() const;
};

} // namespace pas::physics
```

### 4.3 Electromagnetic Field

```cpp
// src/physics/EMField.hpp

#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include <memory>

namespace pas::physics {

// Field evaluation result
struct FieldValue {
    glm::dvec3 E{0.0};  // Electric field (V/m)
    glm::dvec3 B{0.0};  // Magnetic field (Tesla)
};

// Abstract field source
class FieldSource {
public:
    virtual ~FieldSource() = default;

    virtual FieldValue evaluate(const glm::dvec3& position, double time) const = 0;
    virtual glm::dvec3 getBoundingBoxMin() const = 0;
    virtual glm::dvec3 getBoundingBoxMax() const = 0;
    virtual bool isInside(const glm::dvec3& position) const = 0;

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

protected:
    bool m_enabled = true;
};

// Uniform magnetic field (dipole approximation)
class UniformBField : public FieldSource {
public:
    UniformBField(const glm::dvec3& field,
                  const glm::dvec3& center,
                  const glm::dvec3& halfExtents);

    FieldValue evaluate(const glm::dvec3& position, double time) const override;
    glm::dvec3 getBoundingBoxMin() const override;
    glm::dvec3 getBoundingBoxMax() const override;
    bool isInside(const glm::dvec3& position) const override;

    void setField(const glm::dvec3& field) { m_field = field; }
    glm::dvec3 getField() const { return m_field; }

private:
    glm::dvec3 m_field;
    glm::dvec3 m_center;
    glm::dvec3 m_halfExtents;
};

// Quadrupole magnetic field
class QuadrupoleField : public FieldSource {
public:
    QuadrupoleField(double gradient,          // T/m
                    const glm::dvec3& center,
                    double length,
                    double aperture,
                    bool focusing);           // true = horizontal focus

    FieldValue evaluate(const glm::dvec3& position, double time) const override;
    glm::dvec3 getBoundingBoxMin() const override;
    glm::dvec3 getBoundingBoxMax() const override;
    bool isInside(const glm::dvec3& position) const override;

    void setGradient(double gradient) { m_gradient = gradient; }
    double getGradient() const { return m_gradient; }

private:
    double m_gradient;
    glm::dvec3 m_center;
    double m_length;
    double m_aperture;
    bool m_focusing;
};

// Oscillating RF electric field
class RFField : public FieldSource {
public:
    RFField(double voltage,                   // Peak voltage (V)
            double frequency,                  // Hz
            double phase,                      // radians
            const glm::dvec3& center,
            double gapLength,
            double aperture);

    FieldValue evaluate(const glm::dvec3& position, double time) const override;
    glm::dvec3 getBoundingBoxMin() const override;
    glm::dvec3 getBoundingBoxMax() const override;
    bool isInside(const glm::dvec3& position) const override;

    void setVoltage(double voltage) { m_voltage = voltage; }
    void setFrequency(double frequency) { m_frequency = frequency; }
    void setPhase(double phase) { m_phase = phase; }

private:
    double m_voltage;
    double m_frequency;
    double m_phase;
    glm::dvec3 m_center;
    double m_gapLength;
    double m_aperture;
};

// Composite field manager
class EMFieldManager {
public:
    void addSource(std::shared_ptr<FieldSource> source);
    void removeSource(const std::shared_ptr<FieldSource>& source);
    void clear();

    FieldValue evaluateTotal(const glm::dvec3& position, double time) const;

    const std::vector<std::shared_ptr<FieldSource>>& getSources() const;

private:
    std::vector<std::shared_ptr<FieldSource>> m_sources;
};

} // namespace pas::physics
```

### 4.4 Numerical Integrators

```cpp
// src/physics/Integrator.hpp

#pragma once

#include "Particle.hpp"
#include "EMField.hpp"
#include <functional>

namespace pas::physics {

// Lorentz force: F = q(E + v x B)
using ForceFunction = std::function<glm::dvec3(const Particle&, double time)>;

// Abstract integrator interface
class Integrator {
public:
    virtual ~Integrator() = default;

    virtual void step(Particle& particle,
                      const EMFieldManager& fields,
                      double time,
                      double dt) = 0;

    virtual std::string getName() const = 0;
    virtual int getOrder() const = 0;
};

// Euler method (1st order, for reference/debugging)
class EulerIntegrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fields,
              double time,
              double dt) override;

    std::string getName() const override { return "Euler"; }
    int getOrder() const override { return 1; }
};

// Velocity Verlet (2nd order, symplectic)
class VelocityVerletIntegrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fields,
              double time,
              double dt) override;

    std::string getName() const override { return "Velocity Verlet"; }
    int getOrder() const override { return 2; }
};

// 4th order Runge-Kutta
class RK4Integrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fields,
              double time,
              double dt) override;

    std::string getName() const override { return "RK4"; }
    int getOrder() const override { return 4; }
};

// Boris pusher (for magnetic fields, preserves phase space)
class BorisIntegrator : public Integrator {
public:
    void step(Particle& particle,
              const EMFieldManager& fields,
              double time,
              double dt) override;

    std::string getName() const override { return "Boris"; }
    int getOrder() const override { return 2; }
};

// Adaptive RK45 (Dormand-Prince)
class RK45Integrator : public Integrator {
public:
    struct Config {
        double tolerance = 1e-9;
        double minStep = 1e-15;
        double maxStep = 1e-6;
        double safetyFactor = 0.9;
    };

    explicit RK45Integrator(const Config& config = Config{});

    void step(Particle& particle,
              const EMFieldManager& fields,
              double time,
              double dt) override;

    std::string getName() const override { return "RK45 (Adaptive)"; }
    int getOrder() const override { return 5; }

    double getLastStepSize() const { return m_lastStepSize; }

private:
    Config m_config;
    double m_lastStepSize = 0.0;
};

} // namespace pas::physics
```

### 4.5 Particle System

```cpp
// src/physics/ParticleSystem.hpp

#pragma once

#include "Particle.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace pas::physics {

// Beam distribution types
enum class DistributionType {
    Uniform,
    Gaussian,
    Waterbag,
    KV,          // Kapchinsky-Vladimirsky
    Custom
};

// Beam parameters
struct BeamParameters {
    ParticleType particleType = ParticleType::Proton;
    int numParticles = 1000;

    // Energy
    double kineticEnergy = 1.0e9 * constants::eV;  // 1 GeV default
    double energySpread = 0.001;                    // dE/E (relative)

    // Transverse emittance (normalized, m*rad)
    double emittanceX = 1.0e-6;
    double emittanceY = 1.0e-6;

    // Bunch length
    double bunchLength = 0.01;  // meters (RMS)

    // Initial position and direction
    glm::dvec3 position{0.0};
    glm::dvec3 direction{0.0, 0.0, 1.0};

    // Distribution type
    DistributionType distribution = DistributionType::Gaussian;
};

// Beam statistics
struct BeamStatistics {
    // Centroids
    glm::dvec3 meanPosition{0.0};
    glm::dvec3 meanMomentum{0.0};

    // RMS sizes
    glm::dvec3 rmsPosition{0.0};
    glm::dvec3 rmsMomentum{0.0};

    // Emittances
    double emittanceX = 0.0;
    double emittanceY = 0.0;
    double emittanceZ = 0.0;

    // Energy
    double meanEnergy = 0.0;
    double rmsEnergySpread = 0.0;

    // Particle counts
    int totalParticles = 0;
    int activeParticles = 0;
    int lostParticles = 0;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    // Particle management
    void generateBeam(const BeamParameters& params);
    void injectParticle(const Particle& particle);
    void clear();
    void removeInactive();

    // Accessors
    std::vector<Particle>& getParticles();
    const std::vector<Particle>& getParticles() const;
    size_t getParticleCount() const;
    size_t getActiveCount() const;

    // Statistics
    BeamStatistics computeStatistics() const;

    // Loss tracking
    using LossCallback = std::function<void(const Particle&, const std::string&)>;
    void setLossCallback(LossCallback callback);
    void markParticleLost(size_t index, const std::string& reason);

    // Iteration
    template<typename Func>
    void forEachActive(Func&& func);

    template<typename Func>
    void forEachActiveParallel(Func&& func);

private:
    std::vector<Particle> m_particles;
    uint32_t m_nextId = 0;
    LossCallback m_lossCallback;

    void generateGaussianBeam(const BeamParameters& params);
    void generateUniformBeam(const BeamParameters& params);
    void generateWaterbagBeam(const BeamParameters& params);
};

} // namespace pas::physics
```

### 4.6 Physics Engine

```cpp
// src/physics/PhysicsEngine.hpp

#pragma once

#include "ParticleSystem.hpp"
#include "EMField.hpp"
#include "Integrator.hpp"
#include <memory>
#include <string>

namespace pas::physics {

class PhysicsEngine {
public:
    struct Config {
        double timeStep = 1e-12;        // seconds
        int substeps = 1;                // subdivisions per frame
        std::string integratorType = "Boris";
        bool relativisticCorrections = true;
        bool spaceChargeEnabled = false;
        double spaceChargeFactor = 1.0;
    };

    PhysicsEngine();
    explicit PhysicsEngine(const Config& config);
    ~PhysicsEngine();

    // Simulation control
    void update(double dt);
    void reset();

    // Configuration
    void setConfig(const Config& config);
    const Config& getConfig() const;
    void setTimeStep(double dt);
    void setIntegrator(const std::string& type);

    // Components
    ParticleSystem& getParticleSystem();
    EMFieldManager& getFieldManager();

    // Aperture/boundary checking
    void setApertureCheck(std::function<bool(const glm::dvec3&)> check);

    // Statistics
    double getSimulationTime() const { return m_simulationTime; }
    int getTotalSteps() const { return m_totalSteps; }
    double getAverageStepTime() const;

private:
    Config m_config;
    ParticleSystem m_particleSystem;
    EMFieldManager m_fieldManager;
    std::unique_ptr<Integrator> m_integrator;

    double m_simulationTime = 0.0;
    int m_totalSteps = 0;
    double m_accumulatedStepTime = 0.0;

    std::function<bool(const glm::dvec3&)> m_apertureCheck;

    void stepParticle(Particle& particle, double dt);
    void checkAperture(Particle& particle);
    void applySpaceCharge();
    std::unique_ptr<Integrator> createIntegrator(const std::string& type);
};

} // namespace pas::physics
```

### 4.7 Simulation Step Pipeline

1. Accumulate frame `dt` into a fixed-step accumulator.
2. For each fixed step, run `substeps` of size `timeStep / substeps`:
   - Evaluate external fields at the current time.
   - Advance particles with the active integrator.
   - Update derived quantities (gamma, beta, energies).
   - Apply aperture checks and loss callbacks.
   - Apply space-charge kick if enabled.
3. Advance `simulationTime` by the fixed step size and update statistics.

### 4.8 Space Charge Model

- Initial implementation uses a simplified mean-field kick based on the instantaneous beam distribution.
- The kick is scaled by `spaceChargeFactor` and applied after external fields within each substep.
- Image charges, wakefields, and boundary effects are out of scope for v1.0.0.

---

## 5. Rendering System

### 5.1 Shader Class

```cpp
// src/rendering/Shader.hpp

#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace pas::rendering {

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    Shader(const std::string& vertexPath,
           const std::string& geometryPath,
           const std::string& fragmentPath);
    ~Shader();

    // Non-copyable
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Movable
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void bind() const;
    void unbind() const;

    uint32_t getProgram() const { return m_program; }

    // Uniform setters
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat3(const std::string& name, const glm::mat3& value);
    void setMat4(const std::string& name, const glm::mat4& value);
    void setBool(const std::string& name, bool value);

private:
    uint32_t m_program = 0;
    mutable std::unordered_map<std::string, int> m_uniformCache;

    int getUniformLocation(const std::string& name) const;
    uint32_t compileShader(uint32_t type, const std::string& source);
    std::string loadShaderSource(const std::string& path);
};

class ShaderManager {
public:
    static ShaderManager& getInstance();

    Shader* load(const std::string& name,
                 const std::string& vertexPath,
                 const std::string& fragmentPath);
    Shader* load(const std::string& name,
                 const std::string& vertexPath,
                 const std::string& geometryPath,
                 const std::string& fragmentPath);

    Shader* get(const std::string& name);
    void remove(const std::string& name);
    void clear();

private:
    ShaderManager() = default;
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
};

} // namespace pas::rendering
```

### 5.2 Camera System

```cpp
// src/rendering/Camera.hpp

#pragma once

#include <glm/glm.hpp>

namespace pas::rendering {

enum class CameraMode {
    Free,           // Free-fly camera
    Orbit,          // Orbit around target
    Follow,         // Follow beam
    Fixed           // Fixed position and orientation
};

class Camera {
public:
    struct Config {
        float fov = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 10000.0f;
        float moveSpeed = 10.0f;
        float rotateSpeed = 0.1f;
        float zoomSpeed = 5.0f;
        float orbitDistance = 50.0f;
    };

    Camera(float aspectRatio, const Config& config = Config{});

    // Update
    void update(float deltaTime);
    void processInput();

    // Matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;

    // Position and orientation
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getForward() const { return m_forward; }
    glm::vec3 getRight() const { return m_right; }
    glm::vec3 getUp() const { return m_up; }

    void setPosition(const glm::vec3& position);
    void lookAt(const glm::vec3& target);
    void setTarget(const glm::vec3& target);

    // Mode
    void setMode(CameraMode mode);
    CameraMode getMode() const { return m_mode; }

    // Properties
    void setAspectRatio(float ratio);
    void setFOV(float fov);
    void setConfig(const Config& config);

    // Orbit controls
    void orbit(float deltaYaw, float deltaPitch);
    void zoom(float delta);
    void pan(float deltaX, float deltaY);

private:
    void updateVectors();
    void updateOrbit();

    Config m_config;
    CameraMode m_mode = CameraMode::Free;

    glm::vec3 m_position{0.0f, 10.0f, 50.0f};
    glm::vec3 m_target{0.0f};
    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_aspectRatio;

    // Orbit state
    float m_orbitYaw = 0.0f;
    float m_orbitPitch = 30.0f;
    float m_orbitDistance;

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};
    bool m_viewDirty = true;
    bool m_projectionDirty = true;
};

} // namespace pas::rendering
```

### 5.3 Mesh and Geometry

```cpp
// src/rendering/Mesh.hpp

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace pas::rendering {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color{1.0f};
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<uint32_t>& indices);
    ~Mesh();

    // Non-copyable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Movable
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;
    void drawInstanced(int instanceCount) const;

    size_t getVertexCount() const { return m_vertexCount; }
    size_t getIndexCount() const { return m_indexCount; }

    uint32_t getVAO() const { return m_vao; }

private:
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ebo = 0;
    size_t m_vertexCount = 0;
    size_t m_indexCount = 0;
};

// Factory for common geometries
class MeshFactory {
public:
    static std::unique_ptr<Mesh> createCube(float size = 1.0f);
    static std::unique_ptr<Mesh> createSphere(float radius = 1.0f, int segments = 32);
    static std::unique_ptr<Mesh> createCylinder(float radius = 1.0f,
                                                 float height = 1.0f,
                                                 int segments = 32);
    static std::unique_ptr<Mesh> createTorus(float majorRadius = 1.0f,
                                              float minorRadius = 0.3f,
                                              int majorSegments = 48,
                                              int minorSegments = 24);
    static std::unique_ptr<Mesh> createTube(float innerRadius,
                                             float outerRadius,
                                             float length,
                                             int segments = 32);
    static std::unique_ptr<Mesh> createPlane(float width = 1.0f,
                                              float height = 1.0f);
    static std::unique_ptr<Mesh> createGrid(float size = 100.0f,
                                             int divisions = 20);

    // Accelerator specific
    static std::unique_ptr<Mesh> createBeamPipe(float radius,
                                                 float length,
                                                 int segments = 32);
    static std::unique_ptr<Mesh> createDipoleMagnet(float length,
                                                     float width,
                                                     float height,
                                                     float gapHeight);
    static std::unique_ptr<Mesh> createQuadrupoleMagnet(float length,
                                                         float aperture);
};

} // namespace pas::rendering
```

### 5.4 Particle Renderer

```cpp
// src/rendering/ParticleRenderer.hpp

#pragma once

#include "../physics/ParticleSystem.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <glm/glm.hpp>

namespace pas::rendering {

class ParticleRenderer {
public:
    struct Config {
        float particleSize = 0.1f;
        float minSize = 0.05f;
        float maxSize = 0.5f;
        bool sizeByEnergy = true;
        bool colorByEnergy = true;
        bool showTrails = false;
        int trailLength = 50;
        float trailFade = 0.95f;
        glm::vec3 lowEnergyColor{0.0f, 0.5f, 1.0f};   // Blue
        glm::vec3 highEnergyColor{1.0f, 0.2f, 0.0f};  // Red
        float bloomIntensity = 1.0f;
        bool billboardParticles = true;
    };

    ParticleRenderer();
    ~ParticleRenderer();

    void initialize();
    void render(const physics::ParticleSystem& particles,
                const Camera& camera,
                double simulationTime);
    void renderTrails(const Camera& camera);

    void setConfig(const Config& config);
    const Config& getConfig() const { return m_config; }

    // Performance
    void setMaxRenderedParticles(int count);
    int getRenderedParticleCount() const;

private:
    Config m_config;

    // GPU buffers
    uint32_t m_vao = 0;
    uint32_t m_positionVBO = 0;
    uint32_t m_colorVBO = 0;
    uint32_t m_sizeVBO = 0;

    // Trail storage
    struct TrailPoint {
        glm::vec3 position;
        float alpha;
    };
    std::vector<std::vector<TrailPoint>> m_trails;
    uint32_t m_trailVAO = 0;
    uint32_t m_trailVBO = 0;

    // Staging buffers
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec4> m_colors;
    std::vector<float> m_sizes;

    Shader* m_particleShader = nullptr;
    Shader* m_trailShader = nullptr;

    int m_maxParticles = 100000;
    int m_renderedCount = 0;

    double m_energyMin = 0.0;
    double m_energyMax = 1e12;

    void updateBuffers(const physics::ParticleSystem& particles);
    void updateTrails(const physics::ParticleSystem& particles);
    glm::vec3 energyToColor(double energy) const;
    float energyToSize(double energy) const;
};

} // namespace pas::rendering
```

### 5.5 Accelerator Renderer

```cpp
// src/rendering/AcceleratorRenderer.hpp

#pragma once

#include "../accelerator/Accelerator.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

namespace pas::rendering {

class AcceleratorRenderer {
public:
    struct Config {
        bool showBeamPipe = true;
        bool showMagnets = true;
        bool showRFCavities = true;
        bool showDetectors = true;
        bool showLabels = false;
        bool wireframeMode = false;
        float beamPipeOpacity = 0.3f;
        glm::vec3 beamPipeColor{0.3f, 0.3f, 0.4f};
        glm::vec3 dipoleColor{0.0f, 0.4f, 0.8f};
        glm::vec3 quadrupoleColor{0.8f, 0.2f, 0.2f};
        glm::vec3 rfCavityColor{0.8f, 0.6f, 0.0f};
    };

    AcceleratorRenderer();
    ~AcceleratorRenderer();

    void initialize();
    void buildGeometry(const accelerator::Accelerator& accelerator);
    void render(const Camera& camera);

    void setConfig(const Config& config);
    const Config& getConfig() const { return m_config; }

    // Highlighting
    void setHighlightedComponent(const std::string& name);
    void clearHighlight();

private:
    Config m_config;

    struct ComponentMesh {
        std::unique_ptr<Mesh> mesh;
        glm::mat4 transform;
        glm::vec3 color;
        std::string type;
        std::string name;
    };

    std::vector<ComponentMesh> m_components;
    std::string m_highlightedComponent;

    Shader* m_solidShader = nullptr;
    Shader* m_transparentShader = nullptr;

    void createBeamPipeMesh(const accelerator::BeamPipe& pipe);
    void createDipoleMesh(const accelerator::Dipole& dipole);
    void createQuadrupoleMesh(const accelerator::Quadrupole& quad);
    void createRFCavityMesh(const accelerator::RFCavity& cavity);
};

} // namespace pas::rendering
```

### 5.6 Field Visualizer

```cpp
// src/rendering/FieldVisualizer.hpp

#pragma once

#include "../physics/EMField.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>

namespace pas::rendering {

class FieldVisualizer {
public:
    enum class VisualizationMode {
        None,
        Arrows,           // 3D arrows showing field direction
        Streamlines,      // Field line traces
        ColorMap,         // Color-coded magnitude on plane
        ContourLines      // Iso-magnitude contours
    };

    struct Config {
        VisualizationMode mode = VisualizationMode::None;
        bool showElectric = true;
        bool showMagnetic = true;
        int gridResolutionX = 20;
        int gridResolutionY = 20;
        int gridResolutionZ = 20;
        float arrowScale = 1.0f;
        float arrowMaxLength = 2.0f;
        glm::vec3 slicePlaneNormal{0.0f, 1.0f, 0.0f};
        float slicePlaneOffset = 0.0f;
        glm::vec3 electricColor{1.0f, 0.8f, 0.0f};
        glm::vec3 magneticColor{0.0f, 0.8f, 1.0f};
    };

    FieldVisualizer();
    ~FieldVisualizer();

    void initialize();
    void update(const physics::EMFieldManager& fields,
                double time,
                const glm::vec3& minBounds,
                const glm::vec3& maxBounds);
    void render(const Camera& camera);

    void setConfig(const Config& config);
    const Config& getConfig() const { return m_config; }

private:
    Config m_config;

    // Arrow visualization
    struct Arrow {
        glm::vec3 position;
        glm::vec3 direction;
        float magnitude;
        bool isElectric;
    };
    std::vector<Arrow> m_arrows;
    uint32_t m_arrowVAO = 0;
    uint32_t m_arrowVBO = 0;
    uint32_t m_arrowInstanceVBO = 0;

    // Streamlines
    std::vector<std::vector<glm::vec3>> m_streamlines;
    uint32_t m_streamlineVAO = 0;
    uint32_t m_streamlineVBO = 0;

    Shader* m_arrowShader = nullptr;
    Shader* m_streamlineShader = nullptr;

    void generateArrows(const physics::EMFieldManager& fields, double time);
    void generateStreamlines(const physics::EMFieldManager& fields, double time);
    void traceStreamline(const physics::EMFieldManager& fields,
                         const glm::vec3& start,
                         double time,
                         bool magnetic);
};

} // namespace pas::rendering
```

### 5.7 Main Renderer

```cpp
// src/rendering/Renderer.hpp

#pragma once

#include "Camera.hpp"
#include "ParticleRenderer.hpp"
#include "AcceleratorRenderer.hpp"
#include "FieldVisualizer.hpp"
#include "PostProcessor.hpp"
#include "../physics/PhysicsEngine.hpp"
#include "../accelerator/Accelerator.hpp"
#include <memory>

namespace pas::rendering {

class Renderer {
public:
    struct Config {
        bool enableMSAA = true;
        int msaaSamples = 4;
        bool enableBloom = true;
        bool enableSSAO = false;
        bool enableShadows = false;
        bool showGrid = true;
        bool showAxes = true;
        bool showStats = true;
        glm::vec3 backgroundColor{0.02f, 0.02f, 0.05f};
        glm::vec3 ambientLight{0.2f};
    };

    Renderer();
    ~Renderer();

    void initialize(int width, int height);
    void resize(int width, int height);
    void render(const physics::PhysicsEngine& physics,
                const accelerator::Accelerator& accelerator,
                double simulationTime);

    Camera& getCamera() { return *m_camera; }
    const Camera& getCamera() const { return *m_camera; }

    ParticleRenderer& getParticleRenderer() { return *m_particleRenderer; }
    AcceleratorRenderer& getAcceleratorRenderer() { return *m_acceleratorRenderer; }
    FieldVisualizer& getFieldVisualizer() { return *m_fieldVisualizer; }

    void setConfig(const Config& config);
    const Config& getConfig() const { return m_config; }

private:
    Config m_config;
    int m_width = 0;
    int m_height = 0;

    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<ParticleRenderer> m_particleRenderer;
    std::unique_ptr<AcceleratorRenderer> m_acceleratorRenderer;
    std::unique_ptr<FieldVisualizer> m_fieldVisualizer;
    std::unique_ptr<PostProcessor> m_postProcessor;

    // Framebuffers
    uint32_t m_mainFBO = 0;
    uint32_t m_colorTexture = 0;
    uint32_t m_depthTexture = 0;

    // Grid
    std::unique_ptr<Mesh> m_gridMesh;
    Shader* m_gridShader = nullptr;

    void setupFramebuffer();
    void renderGrid();
    void renderAxes();
};

} // namespace pas::rendering
```

---

## 6. Particle System (Rendering GPU Buffer Management)

### 6.1 GPU Particle Buffer

```cpp
// src/rendering/GPUParticleBuffer.hpp

#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace pas::rendering {

// GPU-side particle data (compact for performance)
struct GPUParticle {
    glm::vec4 positionAndSize;   // xyz = position, w = size
    glm::vec4 colorAndEnergy;    // rgb = color, a = energy (for shader effects)
    glm::vec4 velocityAndAge;    // xyz = velocity (normalized), w = age
};

class GPUParticleBuffer {
public:
    GPUParticleBuffer(size_t maxParticles);
    ~GPUParticleBuffer();

    // Non-copyable
    GPUParticleBuffer(const GPUParticleBuffer&) = delete;
    GPUParticleBuffer& operator=(const GPUParticleBuffer&) = delete;

    void bind() const;
    void unbind() const;

    void updateData(const GPUParticle* data, size_t count);
    void updateSubData(const GPUParticle* data, size_t offset, size_t count);

    size_t getMaxParticles() const { return m_maxParticles; }
    size_t getCurrentCount() const { return m_currentCount; }
    uint32_t getVAO() const { return m_vao; }

private:
    size_t m_maxParticles;
    size_t m_currentCount = 0;

    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
};

} // namespace pas::rendering
```

---

## 7. Accelerator Components

### 7.1 Component Base Class

```cpp
// src/accelerator/Component.hpp

#pragma once

#include <glm/glm.hpp>
#include <string>
#include "../physics/EMField.hpp"

namespace pas::accelerator {

enum class ComponentType {
    BeamPipe,
    Dipole,
    Quadrupole,
    Sextupole,  // Reserved for future (not implemented in v1.0.0)
    RFCavity,
    Collimator, // Reserved for future (not implemented in v1.0.0)
    Detector,
    Drift,      // Field-free element (no FieldSource)
    Custom
};

class Component {
public:
    virtual ~Component() = default;

    // Identification
    virtual ComponentType getType() const = 0;
    virtual std::string getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    // Geometry
    virtual glm::dvec3 getPosition() const { return m_position; }
    virtual glm::dvec3 getRotation() const { return m_rotation; }
    virtual double getLength() const { return m_length; }
    virtual double getAperture() const { return m_aperture; }

    void setPosition(const glm::dvec3& pos) { m_position = pos; }
    void setRotation(const glm::dvec3& rot) { m_rotation = rot; }

    // Field contribution
    virtual std::shared_ptr<physics::FieldSource> getFieldSource() const = 0;

    // Aperture checking
    virtual bool isInsideAperture(const glm::dvec3& localPos) const;
    glm::dvec3 toLocal(const glm::dvec3& globalPos) const;
    glm::dvec3 toGlobal(const glm::dvec3& localPos) const;

    // Lattice position (s coordinate)
    double getSPosition() const { return m_sPosition; }
    void setSPosition(double s) { m_sPosition = s; }

protected:
    std::string m_name;
    glm::dvec3 m_position{0.0};
    glm::dvec3 m_rotation{0.0};  // Euler angles (radians)
    double m_length = 0.0;
    double m_aperture = 0.05;    // Default 5cm aperture
    double m_sPosition = 0.0;    // Position along beam path
};

} // namespace pas::accelerator
```

### 7.2 Beam Pipe

```cpp
// src/accelerator/BeamPipe.hpp

#pragma once

#include "Component.hpp"

namespace pas::accelerator {

class BeamPipe : public Component {
public:
    struct Config {
        double innerRadius = 0.05;   // meters
        double outerRadius = 0.06;
        double length = 1.0;
        glm::dvec3 position{0.0};
        glm::dvec3 rotation{0.0};
    };

    explicit BeamPipe(const Config& config);

    ComponentType getType() const override { return ComponentType::BeamPipe; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getInnerRadius() const { return m_innerRadius; }
    double getOuterRadius() const { return m_outerRadius; }
    bool isInsideAperture(const glm::dvec3& localPos) const override;

private:
    double m_innerRadius;
    double m_outerRadius;
};

} // namespace pas::accelerator
```

### 7.3 Dipole Magnet

```cpp
// src/accelerator/Dipole.hpp

#pragma once

#include "Component.hpp"

namespace pas::accelerator {

class Dipole : public Component {
public:
    struct Config {
        double field = 1.0;          // Tesla
        double length = 2.0;         // meters
        double bendingAngle = 0.0;   // radians (alternative to field)
        double gapHeight = 0.06;     // meters
        double width = 0.5;          // meters
        double aperture = 0.05;
        glm::dvec3 position{0.0};
        glm::dvec3 rotation{0.0};
        bool useAngle = false;       // If true, compute field from angle
        double referenceEnergy = 1e9 * physics::constants::eV;
    };

    explicit Dipole(const Config& config);

    ComponentType getType() const override { return ComponentType::Dipole; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getField() const { return m_field; }
    void setField(double field);
    double getBendingAngle() const { return m_bendingAngle; }
    double getBendingRadius() const;

    double getGapHeight() const { return m_gapHeight; }
    double getWidth() const { return m_width; }

private:
    double m_field;
    double m_bendingAngle;
    double m_gapHeight;
    double m_width;
    double m_referenceEnergy;
    bool m_useAngle;

    std::shared_ptr<physics::UniformBField> m_fieldSource;

    void computeFieldFromAngle();
};

} // namespace pas::accelerator
```

### 7.4 Quadrupole Magnet

```cpp
// src/accelerator/Quadrupole.hpp

#pragma once

#include "Component.hpp"

namespace pas::accelerator {

class Quadrupole : public Component {
public:
    struct Config {
        double gradient = 10.0;      // T/m
        double length = 0.5;         // meters
        double aperture = 0.05;
        bool focusing = true;        // true = horizontal focus (F), false = defocus (D)
        double k1 = 0.0;             // Alternative: normalized strength (1/m^2)
        bool useK1 = false;
        double referenceEnergy = 1e9 * physics::constants::eV;
        glm::dvec3 position{0.0};
        glm::dvec3 rotation{0.0};
    };

    explicit Quadrupole(const Config& config);

    ComponentType getType() const override { return ComponentType::Quadrupole; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getGradient() const { return m_gradient; }
    void setGradient(double gradient);
    double getK1() const { return m_k1; }
    bool isFocusing() const { return m_focusing; }

private:
    double m_gradient;
    double m_k1;
    bool m_focusing;
    double m_referenceEnergy;

    std::shared_ptr<physics::QuadrupoleField> m_fieldSource;

    void computeGradientFromK1();
};

} // namespace pas::accelerator
```

### 7.5 RF Cavity

```cpp
// src/accelerator/RFCavity.hpp

#pragma once

#include "Component.hpp"

namespace pas::accelerator {

class RFCavity : public Component {
public:
    struct Config {
        double voltage = 1e6;        // Volts (peak)
        double frequency = 500e6;    // Hz
        double phase = 0.0;          // radians (relative to synchronous particle)
        double length = 0.5;         // meters
        double aperture = 0.05;
        int harmonicNumber = 1;      // RF harmonic relative to revolution freq
        glm::dvec3 position{0.0};
        glm::dvec3 rotation{0.0};
    };

    explicit RFCavity(const Config& config);

    ComponentType getType() const override { return ComponentType::RFCavity; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    double getVoltage() const { return m_voltage; }
    void setVoltage(double voltage);
    double getFrequency() const { return m_frequency; }
    void setFrequency(double frequency);
    double getPhase() const { return m_phase; }
    void setPhase(double phase);
    int getHarmonicNumber() const { return m_harmonicNumber; }

    // Energy gain for synchronous particle
    double getEnergyGain() const;

private:
    double m_voltage;
    double m_frequency;
    double m_phase;
    int m_harmonicNumber;

    std::shared_ptr<physics::RFField> m_fieldSource;
};

} // namespace pas::accelerator
```

### 7.6 Detector

```cpp
// src/accelerator/Detector.hpp

#pragma once

#include "Component.hpp"
#include "../physics/ParticleSystem.hpp"
#include <functional>
#include <vector>

namespace pas::accelerator {

class Detector : public Component {
public:
    struct Hit {
        uint32_t particleId;
        double time;
        glm::dvec3 position;
        glm::dvec3 momentum;
        double energy;
    };

    struct Config {
        double length = 0.1;
        double aperture = 0.1;
        std::string name = "Detector";
        glm::dvec3 position{0.0};
        glm::dvec3 rotation{0.0};
    };

    using HitCallback = std::function<void(const Hit&)>;

    explicit Detector(const Config& config);

    ComponentType getType() const override { return ComponentType::Detector; }
    std::shared_ptr<physics::FieldSource> getFieldSource() const override;

    // Detection
    void checkParticle(const physics::Particle& particle, double time);
    void setHitCallback(HitCallback callback);

    // Data access
    const std::vector<Hit>& getHits() const { return m_hits; }
    void clearHits();
    size_t getHitCount() const { return m_hits.size(); }

private:
    std::vector<Hit> m_hits;
    HitCallback m_hitCallback;
};

} // namespace pas::accelerator
```

### 7.7 Accelerator (Lattice Container)

```cpp
// src/accelerator/Accelerator.hpp

#pragma once

#include "Component.hpp"
#include "BeamPipe.hpp"
#include "Dipole.hpp"
#include "Quadrupole.hpp"
#include "RFCavity.hpp"
#include "Detector.hpp"
#include "../physics/EMField.hpp"
#include <vector>
#include <memory>
#include <string>

namespace pas::accelerator {

enum class AcceleratorType {
    Linear,      // Linac
    Circular,    // Synchrotron/Storage ring
    Custom
};

class Accelerator {
public:
    struct Config {
        std::string name = "Accelerator";
        AcceleratorType type = AcceleratorType::Circular;
        double circumference = 100.0;    // For circular (meters)
        double totalLength = 100.0;      // For linear (meters)
        double designEnergy = 1e9 * physics::constants::eV;
        physics::ParticleType particleType = physics::ParticleType::Proton;
    };

    Accelerator();
    explicit Accelerator(const Config& config);
    ~Accelerator();

    // Component management
    void addComponent(std::shared_ptr<Component> component);
    void removeComponent(const std::string& name);
    Component* getComponent(const std::string& name);
    const std::vector<std::shared_ptr<Component>>& getComponents() const;

    // Convenience adders
    void addDipole(const Dipole::Config& config);
    void addQuadrupole(const Quadrupole::Config& config);
    void addRFCavity(const RFCavity::Config& config);
    void addDrift(double length);
    void addDetector(const Detector::Config& config);

    // Lattice building
    void buildFODOCell(double cellLength, double quadStrength);
    void closeRing();  // Connect last element to first
    void computeLattice();  // Calculate s-positions and cumulative properties

    // Field integration
    void populateFieldManager(physics::EMFieldManager& manager) const;

    // Aperture
    bool checkAperture(const glm::dvec3& position) const;

    // Properties
    const Config& getConfig() const { return m_config; }
    double getCircumference() const;
    double getRevolutionFrequency(double energy) const;
    double getTunex() const { return m_tuneX; }
    double getTuneY() const { return m_tuneY; }

    // Serialization
    void saveToFile(const std::string& path) const;
    static Accelerator loadFromFile(const std::string& path);

private:
    Config m_config;
    std::vector<std::shared_ptr<Component>> m_components;

    // Lattice properties
    double m_totalLength = 0.0;
    double m_tuneX = 0.0;
    double m_tuneY = 0.0;

    void updateLatticeProperties();
};

} // namespace pas::accelerator
```

---

## 8. User Interface

### 8.1 UI Manager

```cpp
// src/ui/UIManager.hpp

#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

struct ImGuiContext;

namespace pas {

class Application;
class Window;

namespace ui {

class Panel {
public:
    virtual ~Panel() = default;
    virtual void render() = 0;
    virtual std::string getName() const = 0;

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    void toggleVisible() { m_visible = !m_visible; }

protected:
    bool m_visible = true;
};

class UIManager {
public:
    UIManager(Window& window, Application& app);
    ~UIManager();

    void initialize();
    void shutdown();
    void beginFrame();
    void endFrame();
    void render();

    // Panel management
    void addPanel(std::unique_ptr<Panel> panel);
    Panel* getPanel(const std::string& name);

    // Style
    void setDarkTheme();
    void setLightTheme();
    void setAccentColor(float r, float g, float b);

    // Input handling
    bool wantsCaptureKeyboard() const;
    bool wantsCaptureMouse() const;

private:
    Window& m_window;
    Application& m_app;
    ImGuiContext* m_context = nullptr;
    std::vector<std::unique_ptr<Panel>> m_panels;

    void setupStyle();
};

} // namespace ui
} // namespace pas
```

### 8.2 Control Panel

```cpp
// src/ui/ControlPanel.hpp

#pragma once

#include "UIManager.hpp"

namespace pas {

class Application;

namespace physics {
class PhysicsEngine;
}

namespace ui {

class ControlPanel : public Panel {
public:
    ControlPanel(Application& app, physics::PhysicsEngine& physics);

    void render() override;
    std::string getName() const override { return "Control Panel"; }

private:
    Application& m_app;
    physics::PhysicsEngine& m_physics;

    void renderSimulationControls();
    void renderTimeControls();
    void renderPhysicsSettings();
    void renderBeamControls();
};

} // namespace ui
} // namespace pas
```

### 8.3 Diagnostics Panel

```cpp
// src/ui/DiagnosticsPanel.hpp

#pragma once

#include "UIManager.hpp"
#include "../physics/ParticleSystem.hpp"
#include <deque>

namespace pas::ui {

class DiagnosticsPanel : public Panel {
public:
    DiagnosticsPanel(const physics::ParticleSystem& particles);

    void render() override;
    std::string getName() const override { return "Diagnostics"; }

    void update(double simulationTime);

private:
    const physics::ParticleSystem& m_particles;

    // History for plots
    static constexpr int HISTORY_SIZE = 500;
    std::deque<float> m_emittanceXHistory;
    std::deque<float> m_emittanceYHistory;
    std::deque<float> m_energyHistory;
    std::deque<float> m_particleCountHistory;
    std::deque<float> m_timeHistory;

    void renderBeamStatistics();
    void renderPhaseSpacePlot();
    void renderEmittancePlot();
    void renderEnergyDistribution();
    void renderParticleCountPlot();
};

} // namespace pas::ui
```

### 8.4 Settings Panel

```cpp
// src/ui/SettingsPanel.hpp

#pragma once

#include "UIManager.hpp"

namespace pas {

namespace rendering {
class Renderer;
}

namespace ui {

class SettingsPanel : public Panel {
public:
    SettingsPanel(rendering::Renderer& renderer);

    void render() override;
    std::string getName() const override { return "Settings"; }

private:
    rendering::Renderer& m_renderer;

    void renderRenderingSettings();
    void renderCameraSettings();
    void renderParticleSettings();
    void renderFieldVisualization();
    void renderPerformanceSettings();
};

} // namespace ui
} // namespace pas
```

### 8.5 Beam Statistics Panel

```cpp
// src/ui/BeamStatsPanel.hpp

#pragma once

#include "UIManager.hpp"
#include "../physics/ParticleSystem.hpp"

namespace pas::ui {

class BeamStatsPanel : public Panel {
public:
    BeamStatsPanel(const physics::ParticleSystem& particles);

    void render() override;
    std::string getName() const override { return "Beam Statistics"; }

private:
    const physics::ParticleSystem& m_particles;

    void renderPositionStats();
    void renderMomentumStats();
    void renderEmittanceStats();
    void renderEnergyStats();
};

} // namespace pas::ui
```

---

## 9. Data Management

### 9.1 Configuration Manager

```cpp
// src/data/ConfigManager.hpp

#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "../core/Application.hpp"
#include "../physics/PhysicsEngine.hpp"
#include "../accelerator/Accelerator.hpp"
#include "../rendering/Renderer.hpp"

namespace pas::data {

class ConfigManager {
public:
    ConfigManager();

    // Save/Load full configuration
    void saveConfig(const std::string& path,
                    const Application::Config& appConfig,
                    const physics::PhysicsEngine::Config& physicsConfig,
                    const accelerator::Accelerator& accelerator,
                    const rendering::Renderer::Config& renderConfig);

    void loadConfig(const std::string& path,
                    Application::Config& appConfig,
                    physics::PhysicsEngine::Config& physicsConfig,
                    accelerator::Accelerator& accelerator,
                    rendering::Renderer::Config& renderConfig);

    // Save/Load accelerator only
    void saveAccelerator(const std::string& path,
                         const accelerator::Accelerator& accelerator);
    accelerator::Accelerator loadAccelerator(const std::string& path);

    // Presets
    static accelerator::Accelerator createSynchrotronPreset();
    static accelerator::Accelerator createLinacPreset();
    static accelerator::Accelerator createStorageRingPreset();

private:
    nlohmann::json acceleratorToJson(const accelerator::Accelerator& accel);
    void jsonToAccelerator(const nlohmann::json& j, accelerator::Accelerator& accel);
};

} // namespace pas::data
```

### 9.2 Data Logger

```cpp
// src/data/DataLogger.hpp

#pragma once

#include "../physics/ParticleSystem.hpp"
#include <string>
#include <fstream>
#include <vector>

namespace pas::data {

class DataLogger {
public:
    struct Config {
        std::string outputDirectory = "output/";
        std::string prefix = "sim";
        bool logParticles = true;
        bool logStatistics = true;
        int particleLogInterval = 100;    // Steps between logs
        int statisticsLogInterval = 10;
        bool binaryFormat = false;        // Binary vs text
    };

    explicit DataLogger(const Config& config = Config{});
    ~DataLogger();

    void startSession(const std::string& sessionName);
    void endSession();

    void logParticles(const physics::ParticleSystem& particles,
                      double time, int step);
    void logStatistics(const physics::BeamStatistics& stats,
                       double time, int step);
    void logEvent(const std::string& event, double time);

    void flush();

    const Config& getConfig() const { return m_config; }

private:
    Config m_config;
    std::string m_sessionName;
    std::ofstream m_particleFile;
    std::ofstream m_statsFile;
    std::ofstream m_eventFile;
    bool m_sessionActive = false;
};

} // namespace pas::data
```

### 9.3 Export Manager

```cpp
// src/data/ExportManager.hpp

#pragma once

#include "../physics/ParticleSystem.hpp"
#include "../accelerator/Accelerator.hpp"
#include <string>

namespace pas::data {

class ExportManager {
public:
    enum class Format {
        CSV,
        JSON,
        HDF5,
        VTK,
        Binary
    };

    // Export particle data
    static void exportParticles(const physics::ParticleSystem& particles,
                                const std::string& path,
                                Format format = Format::CSV);

    // Export beam statistics history
    static void exportStatistics(const std::vector<physics::BeamStatistics>& history,
                                 const std::string& path,
                                 Format format = Format::CSV);

    // Export phase space distribution
    static void exportPhaseSpace(const physics::ParticleSystem& particles,
                                 const std::string& path,
                                 Format format = Format::CSV);

    // Export accelerator lattice
    static void exportLattice(const accelerator::Accelerator& accel,
                              const std::string& path,
                              Format format = Format::JSON);

    // Export for visualization (VTK format for ParaView)
    static void exportForVisualization(const physics::ParticleSystem& particles,
                                       const std::string& path);

private:
    static void writeCSV(const std::string& path,
                         const std::vector<std::string>& headers,
                         const std::vector<std::vector<double>>& data);
    static void writeJSON(const std::string& path, const nlohmann::json& data);
};

} // namespace pas::data
```

### 9.4 Log and Export Formats

- CSV logs include a header row with units in the column suffix (e.g., `x_m`, `px_kgmps`, `energy_J`).
- Particle CSV (per step) columns: `time_s`, `id`, `x_m`, `y_m`, `z_m`, `vx_mps`, `vy_mps`, `vz_mps`, `px_kgmps`, `py_kgmps`, `pz_kgmps`, `energy_J`, `gamma`, `beta`, `active`.
- Statistics CSV columns: `time_s`, `mean_x_m`, `mean_y_m`, `mean_z_m`, `rms_x_m`, `rms_y_m`, `rms_z_m`, `emit_x_m_rad`, `emit_y_m_rad`, `emit_z_m_rad`, `mean_energy_J`, `rms_energy_spread`.
- Event log columns: `time_s`, `event`, `details` (free text).
- Non-CSV exports should embed units and `schemaVersion` in metadata when supported (HDF5 attributes, VTK field arrays).

---

## 10. Configuration System

### 10.1 Default Configuration Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Particle Accelerator Simulation Configuration",
  "type": "object",
  "properties": {
    "application": {
      "type": "object",
      "properties": {
        "windowTitle": { "type": "string" },
        "windowWidth": { "type": "integer", "minimum": 800 },
        "windowHeight": { "type": "integer", "minimum": 600 },
        "fullscreen": { "type": "boolean" },
        "vsync": { "type": "boolean" },
        "targetFPS": { "type": "integer", "minimum": 30, "maximum": 240 }
      }
    },
    "physics": {
      "type": "object",
      "properties": {
        "timeStep": { "type": "number", "minimum": 1e-15 },
        "substeps": { "type": "integer", "minimum": 1 },
        "integratorType": {
          "type": "string",
          "enum": ["Euler", "VelocityVerlet", "RK4", "Boris", "RK45"]
        },
        "relativisticCorrections": { "type": "boolean" },
        "spaceChargeEnabled": { "type": "boolean" }
      }
    },
    "beam": {
      "type": "object",
      "properties": {
        "particleType": {
          "type": "string",
          "enum": ["Electron", "Positron", "Proton", "Antiproton", "Custom"]
        },
        "numParticles": { "type": "integer", "minimum": 1 },
        "kineticEnergy": { "type": "number", "minimum": 0 },
        "energySpread": { "type": "number", "minimum": 0, "maximum": 1 },
        "emittanceX": { "type": "number", "minimum": 0 },
        "emittanceY": { "type": "number", "minimum": 0 },
        "bunchLength": { "type": "number", "minimum": 0 },
        "distribution": {
          "type": "string",
          "enum": ["Uniform", "Gaussian", "Waterbag", "KV"]
        }
      }
    },
    "accelerator": {
      "type": "object",
      "properties": {
        "name": { "type": "string" },
        "type": {
          "type": "string",
          "enum": ["Linear", "Circular", "Custom"]
        },
        "circumference": { "type": "number", "minimum": 0 },
        "designEnergy": { "type": "number", "minimum": 0 },
        "components": {
          "type": "array",
          "items": { "$ref": "#/definitions/component" }
        }
      }
    },
    "rendering": {
      "type": "object",
      "properties": {
        "msaaSamples": { "type": "integer", "enum": [1, 2, 4, 8] },
        "enableBloom": { "type": "boolean" },
        "enableSSAO": { "type": "boolean" },
        "showGrid": { "type": "boolean" },
        "backgroundColor": { "$ref": "#/definitions/color3" },
        "particleSize": { "type": "number", "minimum": 0.001 },
        "showTrails": { "type": "boolean" },
        "trailLength": { "type": "integer", "minimum": 0 }
      }
    }
  },
  "definitions": {
    "color3": {
      "type": "array",
      "items": { "type": "number", "minimum": 0, "maximum": 1 },
      "minItems": 3,
      "maxItems": 3
    },
    "vector3": {
      "type": "array",
      "items": { "type": "number" },
      "minItems": 3,
      "maxItems": 3
    },
    "component": {
      "type": "object",
      "properties": {
        "type": {
          "type": "string",
          "enum": ["BeamPipe", "Dipole", "Quadrupole", "RFCavity", "Detector", "Drift"]
        },
        "name": { "type": "string" },
        "position": { "$ref": "#/definitions/vector3" },
        "rotation": { "$ref": "#/definitions/vector3" },
        "length": { "type": "number", "minimum": 0 },
        "aperture": { "type": "number", "minimum": 0 }
      },
      "required": ["type"]
    }
  }
}
```

### 10.2 Example Configuration File

```json
{
  "application": {
    "windowTitle": "Particle Accelerator Simulation - LHC Mock",
    "windowWidth": 1920,
    "windowHeight": 1080,
    "fullscreen": false,
    "vsync": true,
    "targetFPS": 60
  },
  "physics": {
    "timeStep": 1e-11,
    "substeps": 10,
    "integratorType": "Boris",
    "relativisticCorrections": true,
    "spaceChargeEnabled": false
  },
  "beam": {
    "particleType": "Proton",
    "numParticles": 10000,
    "kineticEnergy": 7e12,
    "energySpread": 0.0001,
    "emittanceX": 3.75e-6,
    "emittanceY": 3.75e-6,
    "bunchLength": 0.075,
    "distribution": "Gaussian"
  },
  "accelerator": {
    "name": "Mock LHC",
    "type": "Circular",
    "circumference": 26659,
    "designEnergy": 7e12,
    "components": [
      {
        "type": "Dipole",
        "name": "MB.A1",
        "length": 14.3,
        "field": 8.33,
        "aperture": 0.028
      },
      {
        "type": "Quadrupole",
        "name": "MQ.A1",
        "length": 3.1,
        "gradient": 223,
        "aperture": 0.028,
        "focusing": true
      }
    ]
  },
  "rendering": {
    "msaaSamples": 4,
    "enableBloom": true,
    "enableSSAO": false,
    "showGrid": true,
    "backgroundColor": [0.02, 0.02, 0.05],
    "particleSize": 0.05,
    "showTrails": true,
    "trailLength": 100
  }
}
```

---

## 11. Build System

### 11.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(ParticleAcceleratorSimulation VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX /MP)
    add_compile_options("$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
    add_compile_options(-march=native)
endif()

# OpenMP for parallelization
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Dependencies
find_package(OpenGL REQUIRED)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# GLM
add_subdirectory(external/glm)

# Dear ImGui
set(IMGUI_DIR external/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui glfw)

# nlohmann/json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(external/json)

# spdlog
add_subdirectory(external/spdlog)

# Source files
set(SOURCES
    src/main.cpp

    # Core
    src/core/Application.cpp
    src/core/Window.cpp
    src/core/Input.cpp
    src/core/Timer.cpp

    # Physics
    src/physics/PhysicsEngine.cpp
    src/physics/Particle.cpp
    src/physics/ParticleSystem.cpp
    src/physics/EMField.cpp
    src/physics/Integrator.cpp

    # Accelerator
    src/accelerator/Accelerator.cpp
    src/accelerator/BeamPipe.cpp
    src/accelerator/Dipole.cpp
    src/accelerator/Quadrupole.cpp
    src/accelerator/RFCavity.cpp
    src/accelerator/Detector.cpp

    # Rendering
    src/rendering/Renderer.cpp
    src/rendering/Camera.cpp
    src/rendering/Shader.cpp
    src/rendering/Mesh.cpp
    src/rendering/MeshFactory.cpp
    src/rendering/ParticleRenderer.cpp
    src/rendering/AcceleratorRenderer.cpp
    src/rendering/FieldVisualizer.cpp
    src/rendering/PostProcessor.cpp
    src/rendering/GPUParticleBuffer.cpp

    # UI
    src/ui/UIManager.cpp
    src/ui/ControlPanel.cpp
    src/ui/DiagnosticsPanel.cpp
    src/ui/BeamStatsPanel.cpp
    src/ui/SettingsPanel.cpp

    # Data
    src/data/ConfigManager.cpp
    src/data/DataLogger.cpp
    src/data/ExportManager.cpp

    # Utils
    src/utils/Logger.cpp
    src/utils/Random.cpp
    src/utils/ThreadPool.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    glfw
    glad
    glm::glm
    imgui
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
endif()

# Copy resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/config $<TARGET_FILE_DIR:${PROJECT_NAME}>/config
)

# Testing
enable_testing()
add_subdirectory(tests)
```

---

## 12. Testing Strategy

### 12.1 Test Categories

1. **Unit Tests** - Individual component testing
2. **Physics Tests** - Validation against analytical solutions
3. **Integration Tests** - Component interaction
4. **Performance Tests** - Benchmarking and optimization
5. **Visual Tests** - Rendering correctness (manual/screenshot comparison)

### 12.2 Physics Validation Tests

```cpp
// tests/physics/test_integrators.cpp

#include <gtest/gtest.h>
#include "physics/Integrator.hpp"
#include "physics/EMField.hpp"

using namespace pas::physics;

class IntegratorTest : public ::testing::Test {
protected:
    EMFieldManager fields;

    void SetUp() override {
        // Uniform B-field for circular motion test
        auto bField = std::make_shared<UniformBField>(
            glm::dvec3(0.0, 1.0, 0.0),  // 1 Tesla in Y
            glm::dvec3(0.0),
            glm::dvec3(100.0)
        );
        fields.addSource(bField);
    }
};

// Test: Particle in uniform B-field should follow circular path
TEST_F(IntegratorTest, CircularMotionInUniformBField) {
    Particle p(ParticleType::Proton);
    p.position = glm::dvec3(1.0, 0.0, 0.0);
    p.setKineticEnergy(1e6 * constants::eV);  // 1 MeV
    p.velocity = glm::dvec3(0.0, 0.0, p.getSpeed());

    BorisIntegrator integrator;
    double dt = 1e-10;
    double period = 2.0 * M_PI * p.mass * p.gamma / (std::abs(p.charge) * 1.0);
    int steps = static_cast<int>(period / dt);

    glm::dvec3 initialPos = p.position;

    for (int i = 0; i < steps; ++i) {
        integrator.step(p, fields, i * dt, dt);
    }

    // Should return to approximately initial position
    double error = glm::length(p.position - initialPos);
    EXPECT_LT(error, 0.01 * glm::length(initialPos));  // 1% tolerance
}

// Test: Energy conservation in magnetic field
TEST_F(IntegratorTest, EnergyConservationInBField) {
    Particle p(ParticleType::Proton);
    p.position = glm::dvec3(0.0);
    p.setKineticEnergy(100e6 * constants::eV);  // 100 MeV
    p.velocity = glm::dvec3(p.getSpeed(), 0.0, 0.0);

    double initialEnergy = p.totalEnergy;

    RK4Integrator integrator;
    double dt = 1e-11;

    for (int i = 0; i < 10000; ++i) {
        integrator.step(p, fields, i * dt, dt);
    }

    double energyChange = std::abs(p.totalEnergy - initialEnergy) / initialEnergy;
    EXPECT_LT(energyChange, 1e-6);  // Energy should be conserved to 1 ppm
}
```

### 12.3 Performance Benchmarks

```cpp
// tests/performance/benchmark_particle_update.cpp

#include <benchmark/benchmark.h>
#include "physics/PhysicsEngine.hpp"

static void BM_ParticleUpdate_1K(benchmark::State& state) {
    pas::physics::PhysicsEngine engine;
    pas::physics::BeamParameters params;
    params.numParticles = 1000;
    engine.getParticleSystem().generateBeam(params);

    for (auto _ : state) {
        engine.update(1e-10);
    }
}
BENCHMARK(BM_ParticleUpdate_1K);

static void BM_ParticleUpdate_10K(benchmark::State& state) {
    pas::physics::PhysicsEngine engine;
    pas::physics::BeamParameters params;
    params.numParticles = 10000;
    engine.getParticleSystem().generateBeam(params);

    for (auto _ : state) {
        engine.update(1e-10);
    }
}
BENCHMARK(BM_ParticleUpdate_10K);

static void BM_ParticleUpdate_100K(benchmark::State& state) {
    pas::physics::PhysicsEngine engine;
    pas::physics::BeamParameters params;
    params.numParticles = 100000;
    engine.getParticleSystem().generateBeam(params);

    for (auto _ : state) {
        engine.update(1e-10);
    }
}
BENCHMARK(BM_ParticleUpdate_100K);

BENCHMARK_MAIN();
```

---

## Appendix A: Shader Code

### A.1 Particle Vertex Shader

```glsl
// shaders/particle.vert
#version 450 core

layout(location = 0) in vec4 aPositionSize;   // xyz = position, w = size
layout(location = 1) in vec4 aColorEnergy;    // rgb = color, a = energy
layout(location = 2) in vec4 aVelocityAge;    // xyz = velocity, w = age

out VS_OUT {
    vec3 color;
    float size;
    float energy;
    float age;
} vs_out;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uBaseSize;
uniform float uSizeScale;

void main() {
    vec3 worldPos = aPositionSize.xyz;

    // Transform to view space for size calculation
    vec4 viewPos = uView * vec4(worldPos, 1.0);

    gl_Position = uProjection * viewPos;

    // Size attenuation based on distance
    float dist = length(viewPos.xyz);
    float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.01 * dist * dist);

    vs_out.size = aPositionSize.w * uBaseSize * uSizeScale * attenuation;
    vs_out.color = aColorEnergy.rgb;
    vs_out.energy = aColorEnergy.a;
    vs_out.age = aVelocityAge.w;

    gl_PointSize = vs_out.size;
}
```

### A.2 Particle Fragment Shader

```glsl
// shaders/particle.frag
#version 450 core

in VS_OUT {
    vec3 color;
    float size;
    float energy;
    float age;
} fs_in;

out vec4 FragColor;

uniform float uGlowIntensity;
uniform float uMaxAge;

void main() {
    // Circular particle with soft edge
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if (dist > 0.5) {
        discard;
    }

    // Soft edge falloff
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);

    // Age-based fade
    float ageFade = 1.0 - (fs_in.age / uMaxAge);
    ageFade = clamp(ageFade, 0.0, 1.0);

    // Core glow
    float glow = exp(-dist * 6.0) * uGlowIntensity;
    vec3 finalColor = fs_in.color + vec3(glow);

    FragColor = vec4(finalColor, alpha * ageFade);
}
```

### A.3 Accelerator Component Shader

```glsl
// shaders/accelerator.vert
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aColor;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec3 color;
} vs_out;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vs_out.fragPos = worldPos.xyz;
    vs_out.normal = normalize(uNormalMatrix * aNormal);
    vs_out.texCoord = aTexCoord;
    vs_out.color = aColor;

    gl_Position = uProjection * uView * worldPos;
}
```

```glsl
// shaders/accelerator.frag
#version 450 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoord;
    vec3 color;
} fs_in;

out vec4 FragColor;

uniform vec3 uViewPos;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;
uniform float uOpacity;
uniform bool uHighlighted;

void main() {
    // Blinn-Phong lighting
    vec3 normal = normalize(fs_in.normal);
    vec3 lightDir = normalize(-uLightDir);
    vec3 viewDir = normalize(uViewPos - fs_in.fragPos);
    vec3 halfDir = normalize(lightDir + viewDir);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // Specular
    float spec = pow(max(dot(normal, halfDir), 0.0), 32.0);
    vec3 specular = spec * uLightColor * 0.5;

    // Combine
    vec3 result = (uAmbient + diffuse) * fs_in.color + specular;

    // Highlight effect
    if (uHighlighted) {
        result += vec3(0.2, 0.3, 0.5) * (0.5 + 0.5 * sin(gl_FragCoord.x * 0.1));
    }

    FragColor = vec4(result, uOpacity);
}
```

---

## Appendix B: Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Space | Play/Pause simulation |
| R | Reset simulation |
| S | Step simulation (when paused) |
| G | Toggle grid |
| F | Toggle field visualization |
| T | Toggle particle trails |
| P | Toggle control panel |
| D | Toggle diagnostics panel |
| 1-4 | Camera presets |
| F1 | Help overlay |
| F11 | Toggle fullscreen |
| Escape | Exit/Cancel |
| WASD | Camera movement (Free mode) |
| Mouse drag | Orbit camera |
| Scroll | Zoom |
| Middle click + drag | Pan camera |

---

## Appendix C: Performance Targets

| Metric | Target |
|--------|--------|
| Particles simulated | 100,000+ at 60 FPS |
| Physics timestep | 1e-12 seconds |
| Frame time (simulation + render) | < 16ms |
| GPU memory usage | < 500MB |
| Startup time | < 3 seconds |
| Config load time | < 100ms |

---

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-01-01 | Initial specification |
