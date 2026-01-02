# Project Roadmap: Particle Accelerator Simulation

This document outlines the development phases for the Particle Accelerator Simulation, structured to support Test-Driven Development (TDD), Clean Code, and SOLID principles.

**Core Philosophy:**
- **TDD:** Write tests *before* implementation.
- **SOLID:** Rely on abstractions (Interfaces) rather than concrete implementations.
- **Clean Code:** Small, focused classes and functions.
- **Separation of Concerns:** The Physics Engine should not know about the Renderer.

---

## Phase 1: Project Skeleton & Infrastructure
**Goal:** Establish the build system, dependency management, and core utilities.

1.  **Build System Setup**:
    *   Configure CMake.
    *   Integrate dependencies: `spdlog` (logging), `GoogleTest` (testing), `GLM` (math).
    *   Setup folder structure (`src`, `tests`, `assets`).
2.  **Core Utilities**:
    *   Implement `Logger` wrapper (Adapter Pattern).
    *   Implement `Timer` for high-resolution timing.
    *   **TDD Focus:** Verify logging output formats and timer precision.

## Phase 2: Core Physics Domain (The Model)
**Goal:** Implement the mathematical and physical model in isolation. No graphics dependencies.

1.  **Fundamental Types**:
    *   `Constants` and Units.
    *   `Particle` class (State: position, momentum, mass, charge).
    *   **TDD Focus:** Verify relativistic calculations (Gamma, Beta, Energy) against known values.
2.  **Field Abstractions**:
    *   `FieldSource` interface (Open/Closed Principle).
    *   `EMFieldManager` (Composite Pattern).
    *   Implement `UniformBField` and `QuadrupoleField`.
    *   **TDD Focus:** Mock field evaluations and assert correct E/B vectors at specific coordinates.
3.  **Numerical Integrators**:
    *   `Integrator` interface (Strategy Pattern).
    *   Implement `BorisIntegrator` (standard for particles in magnetic fields).
    *   **TDD Focus:** Simulate a particle in a uniform magnetic field and assert it follows a circular path (Cyclotron motion) within tolerance.

## Phase 3: Accelerator Lattice Construction
**Goal:** Model the accelerator components and their arrangement.

1.  **Component System**:
    *   `Component` base class/interface.
    *   Implement `BeamPipe`, `Dipole`, `Quadrupole`, `RFCavity`.
    *   Each component acts as a factory or container for a `FieldSource`.
    *   **TDD Focus:** Verify components correctly report "is inside" and generate the correct field parameters.
2.  **Lattice Management**:
    *   `Accelerator` class (Container).
    *   Logic to position components sequentially (s-coordinate).
    *   **TDD Focus:** Build a simple lattice (e.g., FODO cell) and verify total length and component placement.

## Phase 4: Rendering Engine (The View)
**Goal:** Create the visualization layer.

1.  **Window & Context**:
    *   `Window` class wrapping GLFW.
    *   `Input` handling.
2.  **Shader & Material System**:
    *   `Shader` class.
    *   `ShaderManager`.
3.  **Renderers**:
    *   `Camera` system.
    *   `AcceleratorRenderer`: Renders static meshes (using Instancing if possible).
    *   `ParticleRenderer`: Renders dynamic point data.
    *   **TDD Focus:** Harder to TDD visuals, but we can TDD the *data transformation* logic (e.g., "Given 100 particles, generate the correct vertex buffer data").

## Phase 5: System Integration & Simulation Loop
**Goal:** Connect the Physics Engine to the Application loop.

1.  **Physics Engine Orchestration**:
    *   `PhysicsEngine` class to manage the `ParticleSystem` and time stepping.
2.  **Application Loop**:
    *   Main loop integrating Input -> Update (Physics) -> Render.
    *   **TDD Focus:** Integration tests ensuring the simulation advances time and particles move over frames.

## Phase 6: User Interface (The Controller)
**Goal:** Interactive control and diagnostics.

1.  **ImGui Integration**:
    *   Setup Dear ImGui context.
2.  **Control Panels**:
    *   `ControlPanel`: Start/Stop/Pause, Time scale.
    *   `SettingsPanel`: Modify magnet strengths/RF parameters on the fly.
3.  **Diagnostics**:
    *   `BeamStatsPanel`: Real-time plots of emittance, energy spread.

## Phase 7: Configuration & Polish
**Goal:** Persistence and optimization.

1.  **Configuration System**:
    *   JSON loader for Accelerator configs.
    *   **TDD Focus:** Verify saving/loading preserves all simulation parameters.
2.  **Performance Optimization**:
    *   Parallelize particle updates (OpenMP).
    *   Instanced rendering optimizations.

---
**Next Steps:**
Begin with **Phase 1: Project Skeleton**.
