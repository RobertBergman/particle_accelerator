# 1. Overview & Architecture

## 1.1 Purpose
A real-time, interactive 3D simulation of a particle accelerator using C++ and OpenGL. The simulation models charged particle dynamics through electromagnetic fields, visualizes particle beams, and allows users to interact with accelerator parameters.

## 1.2 Key Features
- Real-time particle physics simulation with relativistic corrections
- 3D visualization using modern OpenGL (4.5+)
- Interactive camera controls and parameter adjustment
- Support for circular (synchrotron) and linear (linac) accelerator configurations
- Beam diagnostics and statistics display
- Configuration save/load functionality
- Performance-optimized for 100,000+ particles

## 1.3 Target Platform
- **OS**: Windows 10/11, Linux
- **Graphics**: OpenGL 4.5+ compatible GPU
- **Memory**: 8GB+ RAM recommended
- **CPU**: Multi-core processor (simulation parallelized via OpenMP/threads)

## 1.4 Dependencies
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

## 1.5 Units, Coordinate System, and Conventions
- Coordinate system is right-handed: +X right, +Y up, +Z forward (linac beamline direction).
- For circular machines, the local beam frame uses (x, y, s) with s along the reference orbit, x radial outward, y vertical.
- Units are SI unless stated: meters, seconds, kilograms, Coulombs, radians; E in V/m and B in Tesla.
- Energies are represented internally in Joules; configuration files specify energies in eV and are converted on load.
- Momentum uses kg*m/s; `beamMomentum.delta` is relative momentum deviation (unitless).

## 1.6 Determinism and Reproducibility
- Runs are reproducible given the same config, random seed, and platform.
- Beam generation and any stochastic effects use a deterministic RNG seeded via config.
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
│   ├── core/           # App, Window, Input, Timer
│   ├── physics/        # Engine, Particles, Fields, Integrators
│   ├── accelerator/    # Components, Lattice
│   ├── rendering/      # OpenGL, Shaders, Camera
│   ├── ui/             # ImGui Panels
│   ├── data/           # Config, Logging
│   └── utils/          # Logger, Math, Random
├── shaders/
├── assets/
├── config/
├── tests/
└── docs/
```

---

## Appendix: Performance Targets
| Metric | Target |
|--------|--------|
| Particles simulated | 100,000+ at 60 FPS |
| Physics timestep | 1e-12 seconds |
| Frame time (simulation + render) | < 16ms |
| GPU memory usage | < 500MB |
| Startup time | < 3 seconds |
| Config load time | < 100ms |

## Appendix: Keyboard Shortcuts
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
