#include "rendering/ParticleRenderer.hpp"
#include "physics/Constants.hpp"
#include "utils/Logger.hpp"

#include <glad/glad.h>
#include <algorithm>

namespace pas::rendering {

// Embedded shaders for particle rendering
static const char* PARTICLE_VERTEX_SHADER = R"(
#version 450 core
layout(location = 0) in vec4 aPositionSize;
layout(location = 1) in vec4 aColorEnergy;

out VS_OUT {
    vec3 color;
    float size;
} vs_out;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uBasePointSize;

void main() {
    vec4 viewPos = uView * vec4(aPositionSize.xyz, 1.0);
    gl_Position = uProjection * viewPos;

    // Size attenuation based on distance
    float dist = length(viewPos.xyz);
    vs_out.size = aPositionSize.w * uBasePointSize * (500.0 / max(dist, 1.0));
    vs_out.color = aColorEnergy.rgb;
    gl_PointSize = max(vs_out.size, 1.0);
}
)";

static const char* PARTICLE_FRAGMENT_SHADER = R"(
#version 450 core
in VS_OUT {
    vec3 color;
    float size;
} fs_in;

out vec4 FragColor;

void main() {
    // Create circular point sprite
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if (dist > 0.5) {
        discard;
    }

    // Soft edge falloff
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);

    // Add glow effect
    vec3 glowColor = fs_in.color * 1.5;
    vec3 finalColor = mix(glowColor, fs_in.color, smoothstep(0.0, 0.3, dist));

    FragColor = vec4(finalColor, alpha);
}
)";

ParticleRenderer::ParticleRenderer() = default;

ParticleRenderer::~ParticleRenderer() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
}

bool ParticleRenderer::init() {
    createShader();
    createBuffers();

    if (!m_shader.isValid()) {
        PAS_ERROR("Failed to create particle shader");
        return false;
    }

    return true;
}

void ParticleRenderer::createShader() {
    m_shader.load(PARTICLE_VERTEX_SHADER, PARTICLE_FRAGMENT_SHADER);
}

void ParticleRenderer::createBuffers() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Position + Size (vec4)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GPUParticle),
                          reinterpret_cast<void*>(offsetof(GPUParticle, positionSize)));

    // Color + Energy (vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GPUParticle),
                          reinterpret_cast<void*>(offsetof(GPUParticle, colorEnergy)));

    glBindVertexArray(0);
}

void ParticleRenderer::update(const physics::ParticleSystem& system) {
    const auto& particles = system.getParticles();
    m_particleCount = particles.size();

    // Find active particles and energy range
    m_gpuParticles.clear();
    m_gpuParticles.reserve(m_particleCount);

    float minE = std::numeric_limits<float>::max();
    float maxE = std::numeric_limits<float>::lowest();

    for (const auto& p : particles) {
        if (!p.isActive()) continue;

        float energy = static_cast<float>(p.getKineticEnergy());
        minE = std::min(minE, energy);
        maxE = std::max(maxE, energy);
    }

    if (m_autoEnergyRange && maxE > minE) {
        m_minEnergy = minE;
        m_maxEnergy = maxE;
    }

    // Build GPU particle data
    for (const auto& p : particles) {
        if (!p.isActive()) continue;

        GPUParticle gp;
        const auto& pos = p.getPosition();
        gp.positionSize = glm::vec4(
            static_cast<float>(pos.x),
            static_cast<float>(pos.y),
            static_cast<float>(pos.z),
            1.0f
        );

        // Calculate color based on scheme
        glm::vec3 color;
        float energy = static_cast<float>(p.getKineticEnergy());
        float normalizedEnergy = 0.5f;

        if (m_maxEnergy > m_minEnergy) {
            normalizedEnergy = (energy - m_minEnergy) / (m_maxEnergy - m_minEnergy);
            normalizedEnergy = std::clamp(normalizedEnergy, 0.0f, 1.0f);
        }

        switch (m_colorScheme) {
            case ParticleColorScheme::ByEnergy:
                color = energyToColor(normalizedEnergy);
                break;
            case ParticleColorScheme::BySpeed: {
                const auto& mom = p.getMomentum();
                float speed = static_cast<float>(glm::length(mom));
                float normalizedSpeed = std::clamp(speed / 1e-18f, 0.0f, 1.0f);
                color = energyToColor(normalizedSpeed);
                break;
            }
            case ParticleColorScheme::ByCharge:
                color = (p.getCharge() > 0) ? glm::vec3(1.0f, 0.2f, 0.2f) : glm::vec3(0.2f, 0.2f, 1.0f);
                break;
            case ParticleColorScheme::Uniform:
            default:
                color = m_uniformColor;
                break;
        }

        gp.colorEnergy = glm::vec4(color, normalizedEnergy);
        m_gpuParticles.push_back(gp);
    }

    m_activeCount = m_gpuParticles.size();
    updateBuffers();
}

void ParticleRenderer::updateBuffers() {
    if (m_gpuParticles.empty()) return;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    size_t requiredSize = m_gpuParticles.size() * sizeof(GPUParticle);
    if (requiredSize > m_bufferCapacity) {
        // Reallocate with some headroom
        m_bufferCapacity = requiredSize * 2;
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_bufferCapacity),
                     nullptr, GL_DYNAMIC_DRAW);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    static_cast<GLsizeiptr>(requiredSize),
                    m_gpuParticles.data());
}

void ParticleRenderer::render(const glm::mat4& view, const glm::mat4& projection) {
    if (m_activeCount == 0 || !m_shader.isValid()) return;

    // Enable point sprite
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Disable depth write for transparent particles
    glDepthMask(GL_FALSE);

    m_shader.use();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", projection);
    m_shader.setFloat("uBasePointSize", m_pointSize);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_activeCount));
    glBindVertexArray(0);

    // Restore state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleRenderer::setEnergyRange(float minEnergy, float maxEnergy) {
    m_minEnergy = minEnergy;
    m_maxEnergy = maxEnergy;
    m_autoEnergyRange = false;
}

glm::vec3 ParticleRenderer::energyToColor(float t) const {
    // Blue -> Cyan -> Green -> Yellow -> Red
    t = std::clamp(t, 0.0f, 1.0f);

    if (t < 0.25f) {
        float f = t / 0.25f;
        return glm::vec3(0.0f, f, 1.0f);  // Blue to Cyan
    } else if (t < 0.5f) {
        float f = (t - 0.25f) / 0.25f;
        return glm::vec3(0.0f, 1.0f, 1.0f - f);  // Cyan to Green
    } else if (t < 0.75f) {
        float f = (t - 0.5f) / 0.25f;
        return glm::vec3(f, 1.0f, 0.0f);  // Green to Yellow
    } else {
        float f = (t - 0.75f) / 0.25f;
        return glm::vec3(1.0f, 1.0f - f, 0.0f);  // Yellow to Red
    }
}

} // namespace pas::rendering
