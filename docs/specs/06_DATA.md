# 6. Data & Configuration

## 6.1 Configuration System (`src/data/ConfigManager.hpp`)
Uses `nlohmann/json` to serialize/deserialize simulation state.

### 6.1.1 JSON Schema
The configuration is split into sections: `application`, `physics`, `beam`, `accelerator`, `rendering`.

**Example Config:**
```json
{
  "physics": {
    "timeStep": 1e-12,
    "integratorType": "Boris"
  },
  "beam": {
    "particleType": "Proton",
    "numParticles": 10000,
    "kineticEnergy": 7e12,
    "distribution": "Gaussian"
  },
  "accelerator": {
    "type": "Circular",
    "circumference": 26659,
    "components": [
      { "type": "Dipole", "length": 14.3, "field": 8.33 },
      { "type": "Quadrupole", "length": 3.1, "gradient": 223 }
    ]
  }
}
```

## 6.2 Data Logging (`src/data/DataLogger.hpp`)
- **CSV Output**: Compatible with Pandas/Excel.
- **Streams**:
    - `particles.csv`: Step-by-step phase space (heavy I/O, optional).
    - `stats.csv`: Aggregated beam stats (lightweight).
- **HDF5 (Future)**: For binary dump of large datasets.

## 6.3 Build System (CMake)

### 6.3.1 Dependencies
- **External libraries** are managed via `add_subdirectory` (vendored) or `find_package`.
- **Primary targets**:
    - `glfw`, `glad` (Rendering)
    - `glm` (Math)
    - `imgui` (UI)
    - `spdlog` (Logging)
    - `nlohmann_json` (Config)
    - `gtest` (Testing)

### 6.3.2 CMake Structure
```cmake
cmake_minimum_required(VERSION 3.20)
project(ParticleAcceleratorSimulation)

# Options
option(BUILD_TESTING "Build unit tests" ON)

# Targets
add_executable(pas src/main.cpp ...)
target_link_libraries(pas PRIVATE ... )

# Post-Build
# Copy shaders/ assets/ config/ to build directory
```
