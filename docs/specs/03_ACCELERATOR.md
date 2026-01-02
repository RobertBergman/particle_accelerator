# 3. Accelerator Components & Lattice

This module defines the physical hardware of the accelerator and their arrangement (the Lattice).

## 3.1 Component Architecture (`src/accelerator/Component.hpp`)
Base abstract class for all beamline elements.

**Interface:**
- `getType()`: Enum (Dipole, Quad, etc.).
- `getFieldSource()`: Returns the underlying `FieldSource` (Physics layer object).
- `isInsideAperture(localPos)`: Geometry check.
- `toLocal(globalPos)` / `toGlobal(localPos)`: Coordinate transformations.
- **Properties**: Length, Position, Rotation, Aperture (radius/size).

## 3.2 Implemented Components

### 3.2.1 Beam Pipe (`BeamPipe.hpp`)
- **Physics**: No field (Drift).
- **Geometry**: Cylindrical tube.
- **Config**: Inner/Outer radius, length.

### 3.2.2 Dipole Magnet (`Dipole.hpp`)
- **Physics**: Uniform vertical B-field (bends beam horizontally).
- **Config**: Field (T) or Bending Angle (rad).
- **Geometry**: Rectangular or Curved Box.

### 3.2.3 Quadrupole Magnet (`Quadrupole.hpp`)
- **Physics**: Linear field gradient ($B_y = G \cdot x$, $B_x = G \cdot y$).
- **Config**: Gradient (T/m) or K1 strength ($m^{-2}$). Focusing or Defocusing.
- **Geometry**: Cylindrical pole tips / Iron yoke.

### 3.2.4 RF Cavity (`RFCavity.hpp`)
- **Physics**: Longitudinal oscillating E-field.
- **Config**: Voltage (Peak), Frequency, Phase lag, Harmonic number.
- **Function**: Adds energy to particles to compensate for synchrotron radiation or to accelerate.

### 3.2.5 Detector (`Detector.hpp`)
- **Physics**: Non-interacting (transparent) or Absorbing.
- **Function**: Records particle phase space coordinates when they pass through.
- **Output**: List of `Hit` structs (time, pos, momentum).

## 3.3 Accelerator / Lattice (`src/accelerator/Accelerator.hpp`)
Container for components.

**Responsibilities:**
- `addComponent(Component)`: Adds element to the line.
- `buildFODOCell(...)`: Helper to create standard periodic structures.
- `closeRing()`: Connects end to start (for Synchrotrons).
- `computeLattice()`: Calculates s-positions (longitudinal coordinate) for all elements.
- `populateFieldManager()`: Registers all component fields with the Physics Engine.

**Lattice Types:**
- **Linear**: Single pass (Linac).
- **Circular**: Periodic boundary conditions (Synchrotron/Storage Ring).
