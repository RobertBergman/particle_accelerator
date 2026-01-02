#include "rendering/Renderer.hpp"
#include "utils/Logger.hpp"

#include <glad/glad.h>

namespace pas::rendering {

Renderer::Renderer() = default;

Renderer::~Renderer() = default;

bool Renderer::init() {
    if (m_initialized) {
        return true;
    }

    PAS_INFO("Initializing renderer");

    setupOpenGL();

    if (!m_particleRenderer.init()) {
        PAS_ERROR("Failed to initialize particle renderer");
        return false;
    }

    if (!m_acceleratorRenderer.init()) {
        PAS_ERROR("Failed to initialize accelerator renderer");
        return false;
    }

    m_initialized = true;
    PAS_INFO("Renderer initialized successfully");
    return true;
}

bool Renderer::initialize(int width, int height) {
    if (!init()) {
        return false;
    }
    resize(width, height);
    return true;
}

void Renderer::setupOpenGL() {
    // Enable depth testing
    if (m_config.enableDepthTest) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    // Enable MSAA
    if (m_config.enableMSAA) {
        glEnable(GL_MULTISAMPLE);
    }

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Set clear color
    glClearColor(m_config.clearColor.r, m_config.clearColor.g,
                 m_config.clearColor.b, m_config.clearColor.a);

    // Enable point size from shader
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void Renderer::resize(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
    glViewport(0, 0, width, height);
    m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_config.wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Renderer::render(const physics::ParticleSystem& particleSystem,
                       const accelerator::Accelerator& accelerator) {
    if (!m_initialized) {
        return;
    }

    // Update camera matrices
    m_camera.update();

    const glm::mat4& view = m_camera.getViewMatrix();
    const glm::mat4& projection = m_camera.getProjectionMatrix();

    // 1. Render accelerator (opaque geometry first)
    m_acceleratorRenderer.render(view, projection);

    // 2. Update and render particles
    m_particleRenderer.update(particleSystem);
    m_particleRenderer.render(view, projection);
}

void Renderer::render(const Camera& camera,
                       const accelerator::Accelerator& accelerator,
                       const physics::ParticleSystem& particleSystem) {
    if (!m_initialized) {
        return;
    }

    const glm::mat4& view = camera.getViewMatrix();
    const glm::mat4& projection = camera.getProjectionMatrix();

    // 1. Render accelerator (opaque geometry first)
    m_acceleratorRenderer.render(view, projection);

    // 2. Update and render particles
    m_particleRenderer.update(particleSystem);
    m_particleRenderer.render(view, projection);
}

void Renderer::endFrame() {
    // Restore polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::setConfig(const RendererConfig& config) {
    m_config = config;

    if (m_initialized) {
        setupOpenGL();
    }
}

void Renderer::setClearColor(const glm::vec4& color) {
    m_config.clearColor = color;
    glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::setWireframeMode(bool enabled) {
    m_config.wireframeMode = enabled;
}

void Renderer::updateAcceleratorGeometry(const accelerator::Accelerator& accelerator) {
    m_acceleratorRenderer.buildGeometry(accelerator);
}

} // namespace pas::rendering
