#pragma once

#include "rendering/Shader.hpp"
#include "physics/ParticleSystem.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace pas::rendering {

/**
 * @brief Particle visualization data for GPU.
 */
struct GPUParticle {
    glm::vec4 positionSize;   // xyz = position, w = size
    glm::vec4 colorEnergy;    // rgb = color, a = normalized energy
};

/**
 * @brief Color scheme for particle visualization.
 */
enum class ParticleColorScheme {
    ByEnergy,    // Blue (low) to Red (high)
    BySpeed,     // Color by velocity magnitude
    ByCharge,    // Positive/negative
    Uniform      // Single color
};

/**
 * @brief Renderer for particle beam visualization.
 *
 * Uses point sprites or instanced billboards for efficient
 * rendering of large particle counts (100k+).
 */
class ParticleRenderer {
public:
    ParticleRenderer();
    ~ParticleRenderer();

    /**
     * @brief Initialize rendering resources.
     * @return True if initialization succeeded.
     */
    bool init();

    /**
     * @brief Update GPU buffer from particle system.
     * @param system The particle system to visualize.
     */
    void update(const physics::ParticleSystem& system);

    /**
     * @brief Render particles.
     * @param viewProjection Combined view-projection matrix.
     */
    void render(const glm::mat4& view, const glm::mat4& projection);

    // Visualization settings
    void setColorScheme(ParticleColorScheme scheme) { m_colorScheme = scheme; }
    ParticleColorScheme getColorScheme() const { return m_colorScheme; }

    void setPointSize(float size) { m_pointSize = size; }
    float getPointSize() const { return m_pointSize; }

    void setUniformColor(const glm::vec3& color) { m_uniformColor = color; }
    const glm::vec3& getUniformColor() const { return m_uniformColor; }

    void setEnergyRange(float minEnergy, float maxEnergy);
    void setAutoEnergyRange(bool enable) { m_autoEnergyRange = enable; }

    // Statistics
    size_t getParticleCount() const { return m_particleCount; }
    size_t getActiveParticleCount() const { return m_activeCount; }

private:
    void createShader();
    void createBuffers();
    void updateBuffers();
    glm::vec3 energyToColor(float normalizedEnergy) const;

    Shader m_shader;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;

    std::vector<GPUParticle> m_gpuParticles;
    size_t m_particleCount = 0;
    size_t m_activeCount = 0;
    size_t m_bufferCapacity = 0;

    ParticleColorScheme m_colorScheme = ParticleColorScheme::ByEnergy;
    float m_pointSize = 5.0f;
    glm::vec3 m_uniformColor{1.0f, 0.5f, 0.0f};

    float m_minEnergy = 0.0f;
    float m_maxEnergy = 1.0f;
    bool m_autoEnergyRange = true;
};

} // namespace pas::rendering
