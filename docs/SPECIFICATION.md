# Particle Accelerator Simulation - Technical Specification

**Version:** 1.0.0

This document serves as the entry point for the technical specification of the Particle Accelerator Simulation project. The specification is modularized into the following documents to facilitate TDD and focused development.

## 📚 Specification Modules

| Module | Description | File |
| :--- | :--- | :--- |
| **1. Overview** | Project goals, architecture, and high-level constraints. | [01_OVERVIEW.md](specs/01_OVERVIEW.md) |
| **2. Physics Engine** | Core mathematical model, particles, fields, and integrators. | [02_PHYSICS.md](specs/02_PHYSICS.md) |
| **3. Accelerator** | Beamline components (Magnets, RF) and lattice construction. | [03_ACCELERATOR.md](specs/03_ACCELERATOR.md) |
| **4. Rendering** | Visualization pipeline, OpenGL implementation, and shaders. | [04_RENDERING.md](specs/04_RENDERING.md) |
| **5. Application** | Main loop, windowing, and User Interface (ImGui). | [05_APPLICATION.md](specs/05_APPLICATION.md) |
| **6. Data & Config** | File formats, JSON schema, and build system. | [06_DATA.md](specs/06_DATA.md) |

## 🛠️ Development Guide

This project follows **Test-Driven Development (TDD)** and **SOLID** principles.
Please refer to the **[Project Roadmap](ROADMAP.md)** for the implementation order and step-by-step development plan.

### Quick Start
1.  **Read** the [Roadmap](ROADMAP.md).
2.  **Start** with [Phase 1: Project Skeleton](ROADMAP.md#phase-1-project-skeleton--infrastructure).
3.  **Reference** [02_PHYSICS.md](specs/02_PHYSICS.md) when building the core engine.