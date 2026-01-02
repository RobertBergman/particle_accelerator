# 5. Application & User Interface

This module handles the main loop, window management, and user interaction.

## 5.1 Core Infrastructure

### 5.1.1 Application Class (`src/core/Application.hpp`)
- **Role**: Root object, manages lifecycle (Init, Run, Shutdown).
- **State Machine**: Stopped, Running, Paused, Stepping.
- **Main Loop**:
    1. `pollEvents()`
    2. `update(dt)` -> Physics Step
    3. `render()`
    4. `swapBuffers()`

### 5.1.2 Window (`src/core/Window.hpp`)
- Wraps GLFW window.
- Handles Context creation.
- Dispatches Input events (Callbacks).

### 5.1.3 Input System (`src/core/Input.hpp`)
- Static polling interface (`isKeyPressed(KEY_SPACE)`).
- Mouse position and delta tracking.

## 5.2 User Interface (Dear ImGui)

### 5.2.1 UI Manager (`src/ui/UIManager.hpp`)
- Initializes ImGui context.
- Manages a stack of `Panel` objects.
- Handles styling (Dark/Light theme).

### 5.2.2 Control Panel (`src/ui/ControlPanel.hpp`)
- **Simulation Control**: Play, Pause, Reset, Step.
- **Time Control**: Time scale slider (0.1x - 100x), Fixed time step setting.
- **Physics**: Toggle Space Charge, Change Integrator.

### 5.2.3 Diagnostics Panel (`src/ui/DiagnosticsPanel.hpp`)
- **Real-time Plotting**:
    - Beam Emittance vs Time.
    - Particle Energy Distribution (Histogram).
    - Phase Space (x vs px).
- **Stats**: Total particles, Lost particles, Mean Energy.

### 5.2.4 Settings Panel (`src/ui/SettingsPanel.hpp`)
- **Rendering**: Toggle Bloom, Shadows, MSAA.
- **Camera**: FOV, Sensitivity.
- **Visuals**: Particle size, Trail length.
