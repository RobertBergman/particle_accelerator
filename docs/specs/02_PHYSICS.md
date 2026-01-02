# 2. Physics Engine

The physics engine is the core domain of the simulation, responsible for advancing the state of the particle system according to electromagnetic laws.

## 2.1 Physical Constants (`src/physics/Constants.hpp`)
```cpp
namespace pas::physics::constants {
    constexpr double c = 299792458.0;              // Speed of light (m/s)
    constexpr double e = 1.602176634e-19;          // Elementary charge (C)
    constexpr double m_e = 9.1093837015e-31;       // Electron mass (kg)
    constexpr double m_p = 1.67262192369e-27;      // Proton mass (kg)
    constexpr double epsilon_0 = 8.8541878128e-12; // Vacuum permittivity (F/m)
    constexpr double mu_0 = 1.25663706212e-6;      // Vacuum permeability (H/m)
    constexpr double k_B = 1.380649e-23;           // Boltzmann constant (J/K)
    constexpr double eV = 1.602176634e-19;         // Electron volt (J)
}
```

## 2.2 Particle Model (`src/physics/Particle.hpp`)
The particle class tracks 6D phase space coordinates (Position `x,y,z` and Momentum `px,py,pz`).

**Key Attributes:**
- **State:** Position (m), Velocity (m/s), Momentum (kg·m/s).
- **Properties:** Mass (kg), Charge (C), Rest Energy (J).
- **Derived:** Gamma (Lorentz factor), Beta (v/c), Kinetic/Total Energy.
- **Beam Coordinates:** Relative `(x, y, s)` and `(px, py, delta)` for lattice physics.

**Responsibilities:**
- `updateDerivedQuantities()`: Recalculate gamma/beta/energy after momentum change.
- `setMomentum()` / `setVelocity()`: Mutators ensuring consistency.

## 2.3 Electromagnetic Fields (`src/physics/EMField.hpp`)
Abstracts field sources. `FieldValue` struct contains `E` (V/m) and `B` (Tesla).

**Interfaces:**
- `FieldSource`: Abstract base. `evaluate(pos, time)`, `getBoundingBox()`.
- `EMFieldManager`: Composite container summing fields from multiple sources.

**Implementations:**
- `UniformBField`: Constant magnetic field (Dipole approx).
- `QuadrupoleField`: Linear gradient magnetic field (Focusing/Defocusing).
- `RFField`: Oscillating electric field (`E = V * cos(wt + phi)`).

## 2.4 Numerical Integrators (`src/physics/Integrator.hpp`)
Strategies for solving `F = q(E + v x B)`.

**Strategies:**
1. **Euler**: 1st order. For testing/comparison only.
2. **Velocity Verlet**: 2nd order symplectic. Good for conservative systems.
3. **RK4**: 4th order Runge-Kutta. High accuracy, 4 evals/step.
4. **Boris Pusher**: *Default*. De-facto standard for plasmas/particle beams. Preserves phase space volume.
5. **RK45**: Adaptive step size (optional).

## 2.5 Particle System (`src/physics/ParticleSystem.hpp`)
Manages the collection of particles (SoA or AoS).

**Responsibilities:**
- `generateBeam(BeamParameters)`: Initialize distributions (Gaussian, Uniform, Waterbag).
- `computeStatistics()`: Calculate emittance, mean energy, RMS sizes.
- Memory management (adding/removing particles).

## 2.6 Physics Engine Controller (`src/physics/PhysicsEngine.hpp`)
Orchestrates the simulation step.

**Update Loop:**
1. Accumulate `dt` into a fixed-step accumulator.
2. While `accumulator >= timeStep`:
    a. Evaluate fields.
    b. Integrate particle motion (position/momentum).
    c. Apply Space Charge kick (if enabled).
    d. Check Apertures (remove lost particles).
    e. Update derived quantities.
    f. `simulationTime += timeStep`.

## 2.7 Testing Strategy
- **Unit Tests**: Verify constants, particle properties, and field evaluations.
- **Physics Validation**:
    - **Cyclotron Motion**: Particle in uniform B-field must follow circular path.
    - **Energy Conservation**: Particle in static B-field must conserve energy.
    - **Drift**: Particle in zero field moves in straight line.
