# 4. Rendering System

The rendering system visualizes the simulation state using OpenGL 4.5+. It is decoupled from the physics engine, reading state only for display.

## 4.1 Core Rendering Classes

### 4.1.1 Shader System (`src/rendering/Shader.hpp`)
- Wrapper around OpenGL Program objects.
- `load(vert, frag)`: Compiles and links shaders.
- Uniform setters (`setMat4`, `setVec3`, etc.) with caching to minimize GL calls.

### 4.1.2 Camera (`src/rendering/Camera.hpp`)
- Manages View and Projection matrices.
- **Modes**:
    - `Free`: Standard WASD flight.
    - `Orbit`: Rotate around a target point.
    - `Follow`: Lock to the beam centroid.
- **Inputs**: Mouse drag to rotate, Scroll to zoom.

### 4.1.3 Mesh & Geometry (`src/rendering/Mesh.hpp`)
- **Vertex Data**: Position, Normal, UV, Color.
- **MeshFactory**: Generates primitives (Cube, Sphere, Cylinder) and complex shapes (Torus for beam pipe, specialized magnet geometries).

## 4.2 Specialized Renderers

### 4.2.1 Particle Renderer (`src/rendering/ParticleRenderer.hpp`)
Optimized for high particle counts (100k+).
- **Technique**: Point Sprites (GL_POINTS) or Instanced Billboards.
- **Data Transfer**: Maps `ParticleSystem` data to GPU buffers (`GPUParticleBuffer`).
- **Features**:
    - Color by Energy (Heatmap: Blue -> Red).
    - Size attenuation (perspective).
    - Optional Trails (Circular buffer of past positions).
    - Glow/Bloom effect via fragment shader.

### 4.2.2 Accelerator Renderer (`src/rendering/AcceleratorRenderer.hpp`)
Draws the static hardware.
- **Technique**: Standard forward rendering.
- **Features**:
    - "Cutaway" view (semi-transparent beam pipe).
    - Color-coded magnets (Dipoles=Blue, Quads=Red).
    - Instanced rendering for repetitive structures (FODO cells).

### 4.2.3 Field Visualizer (`src/rendering/FieldVisualizer.hpp`)
Visual debugging for EM fields.
- **Modes**:
    - `Arrows`: 3D vector field sampling on a grid.
    - `Streamlines`: Traced field lines.
    - `Slices`: 2D color map on a plane.

## 4.3 Main Renderer (`src/rendering/Renderer.hpp`)
- **Pipeline**:
    1. Shadow Pass (optional).
    2. Geometry Pass (Accelerator).
    3. Particle Pass (with depth write off or soft particles).
    4. Field Visualization Pass.
    5. Post-Processing (Bloom, Tone mapping, Gamma correction).

---

## Appendix: Reference Shaders

### Particle Vertex Shader
```glsl
#version 450 core
layout(location = 0) in vec4 aPositionSize;   // xyz = position, w = size
layout(location = 1) in vec4 aColorEnergy;    // rgb = color, a = energy

out VS_OUT { vec3 color; float size; } vs_out;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 viewPos = uView * vec4(aPositionSize.xyz, 1.0);
    gl_Position = uProjection * viewPos;
    
    // Size attenuation
    float dist = length(viewPos.xyz);
    vs_out.size = aPositionSize.w * (500.0 / dist); 
    vs_out.color = aColorEnergy.rgb;
    gl_PointSize = vs_out.size;
}
```
