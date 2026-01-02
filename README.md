# Particle Accelerator Simulation

A real-time 3D particle accelerator simulation with physics-accurate beam dynamics, built with modern C++20 and OpenGL.

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.5-green.svg)
![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)

## Features

- **Relativistic Particle Physics**: Accurate relativistic calculations for particle dynamics (gamma, beta, momentum, energy)
- **Multiple Integrators**: Euler, Velocity Verlet, Boris (phase-space preserving), and RK4 methods
- **Accelerator Components**: Beam pipes, dipole magnets, quadrupole magnets, and RF cavities
- **FODO Lattice Support**: Built-in helper for constructing focusing-defocusing lattices
- **Real-time 3D Visualization**: OpenGL 4.5 rendering with orbit camera controls
- **ImGui Interface**: Interactive control panel for simulation parameters
- **Beam Diagnostics**: Real-time statistics including emittance, energy spread, and phase space plots

## Screenshots

*Run the application to see the simulation in action!*

## Requirements

- **CMake** 3.20+
- **C++20** compatible compiler (MSVC 2022, GCC 11+, Clang 14+)
- **OpenGL** 4.5 capable GPU
- **Git** (for fetching dependencies)

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/particle_accelerator.git
cd particle_accelerator

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run tests
ctest -C Release

# Run the simulation
./bin/Release/pas.exe  # Windows
./bin/pas              # Linux/macOS
```

## Controls

| Key/Action | Function |
|------------|----------|
| **Space** | Start/Pause simulation |
| **R** | Reset simulation |
| **Right-click + Drag** | Orbit camera |
| **Scroll** | Zoom in/out |
| **ESC** | Exit |

## Architecture

```
src/
├── physics/          # Core physics simulation
│   ├── Particle.hpp      # Relativistic particle representation
│   ├── EMField.hpp       # Electromagnetic field sources
│   ├── Integrator.hpp    # Numerical integration methods
│   ├── ParticleSystem.hpp # Beam generation and statistics
│   └── PhysicsEngine.hpp  # Simulation orchestration
├── accelerator/      # Accelerator lattice
│   ├── Component.hpp     # Beam pipes, magnets, cavities
│   └── Accelerator.hpp   # Lattice construction
├── rendering/        # OpenGL visualization
│   ├── Renderer.hpp      # Main rendering pipeline
│   ├── Camera.hpp        # Orbit/fly camera modes
│   └── Shader.hpp        # GLSL shader management
├── ui/               # ImGui panels
│   ├── ControlPanel.hpp  # Simulation controls
│   └── BeamStatsPanel.hpp # Diagnostics display
├── core/             # Window management
└── config/           # JSON configuration
```

## Physics Model

### Particle Dynamics
Particles are tracked using the relativistic equation of motion:

```
dp/dt = q(E + v × B)
```

The Boris integrator is recommended for long-term stability as it preserves phase-space volume.

### Supported Field Types
- **Uniform B-field**: For dipole bending magnets
- **Quadrupole field**: Linear focusing/defocusing
- **RF field**: Time-varying acceleration cavities

### Beam Statistics
- RMS beam size (σx, σy)
- Normalized emittance (εn,x, εn,y)
- Energy spread (δE/E)
- Twiss parameters (when applicable)

## Configuration

Simulation settings can be saved/loaded as JSON:

```json
{
  "simulation": {
    "timeStep": 1e-11,
    "timeScale": 10000,
    "integratorType": 2,
    "particleCount": 1000,
    "beamEnergy": 1e9
  },
  "window": {
    "width": 1600,
    "height": 900,
    "vsync": true
  }
}
```

Accelerator lattices can also be defined in JSON format.

## Dependencies

All dependencies are automatically fetched via CMake FetchContent:

- [spdlog](https://github.com/gabime/spdlog) - Logging
- [GLM](https://github.com/g-truc/glm) - Math library
- [GLFW](https://github.com/glfw/glfw) - Window/input
- [Dear ImGui](https://github.com/ocornut/imgui) - UI
- [nlohmann/json](https://github.com/nlohmann/json) - JSON parsing
- [GoogleTest](https://github.com/google/googletest) - Testing
- [GLAD](https://glad.dav1d.de/) - OpenGL loader (included)

## Testing

The project includes comprehensive unit tests:

```bash
cd build
ctest -C Release --output-on-failure
```

218 tests covering:
- Relativistic physics calculations
- Integrator accuracy and energy conservation
- Beam generation and statistics
- Accelerator component behavior
- Rendering utilities

## License

MIT License - see LICENSE file for details.

## Acknowledgments

- Physics models based on accelerator physics textbooks (Wiedemann, Wille)
- Boris integrator algorithm from plasma physics literature
- LHC parameters used for validation of relativistic calculations
