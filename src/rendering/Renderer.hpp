#pragma once

#include "rendering/Camera.hpp"
#include "rendering/ParticleRenderer.hpp"
#include "rendering/AcceleratorRenderer.hpp"
#include "physics/ParticleSystem.hpp"
#include "accelerator/Accelerator.hpp"
#include <glm/glm.hpp>

namespace pas::rendering {

/**
 * @brief Renderer configuration.
 */
struct RendererConfig {
    glm::vec4 clearColor{0.1f, 0.1f, 0.15f, 1.0f};
    bool enableMSAA = true;
    bool enableDepthTest = true;
    bool wireframeMode = false;
};

/**
 * @brief Main renderer orchestrating all visualization.
 *
 * Manages the rendering pipeline:
 * 1. Geometry pass (accelerator hardware)
 * 2. Particle pass (beam visualization)
 * 3. UI overlay (handled separately by ImGui)
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * @brief Initialize the rendering system.
     * @return True if initialization succeeded.
     */
    bool init();

    /**
     * @brief Initialize the rendering system with viewport size.
     * @param width Viewport width.
     * @param height Viewport height.
     * @return True if initialization succeeded.
     */
    bool initialize(int width, int height);

    /**
     * @brief Resize the viewport.
     * @param width New width.
     * @param height New height.
     */
    void resize(int width, int height);

    /**
     * @brief Begin a new frame.
     */
    void beginFrame();

    /**
     * @brief Render the scene using internal camera.
     * @param particleSystem Particle system to render.
     * @param accelerator Accelerator to render.
     */
    void render(const physics::ParticleSystem& particleSystem,
                const accelerator::Accelerator& accelerator);

    /**
     * @brief Render the scene with external camera.
     * @param camera Camera to use for rendering.
     * @param accelerator Accelerator to render.
     * @param particleSystem Particle system to render.
     */
    void render(const Camera& camera,
                const accelerator::Accelerator& accelerator,
                const physics::ParticleSystem& particleSystem);

    /**
     * @brief End the frame.
     */
    void endFrame();

    // Camera access
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }

    // Renderer access
    ParticleRenderer& getParticleRenderer() { return m_particleRenderer; }
    AcceleratorRenderer& getAcceleratorRenderer() { return m_acceleratorRenderer; }

    // Configuration
    void setConfig(const RendererConfig& config);
    const RendererConfig& getConfig() const { return m_config; }

    void setClearColor(const glm::vec4& color);
    void setWireframeMode(bool enabled);

    // Update accelerator geometry (call when accelerator changes)
    void updateAcceleratorGeometry(const accelerator::Accelerator& accelerator);

private:
    void setupOpenGL();

    Camera m_camera;
    ParticleRenderer m_particleRenderer;
    AcceleratorRenderer m_acceleratorRenderer;
    RendererConfig m_config;

    int m_viewportWidth = 1280;
    int m_viewportHeight = 720;
    bool m_initialized = false;
};

} // namespace pas::rendering
