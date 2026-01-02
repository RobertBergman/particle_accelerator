#include "rendering/AcceleratorRenderer.hpp"
#include "utils/Logger.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace pas::rendering {

// Solid shader for components
static const char* SOLID_VERTEX_SHADER = R"(
#version 450 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec4 Color;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform vec4 uColor;

void main() {
    FragPos = vec3(uModel * vec4(aPosition, 1.0));
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    Color = uColor;
    gl_Position = uProjection * uView * vec4(FragPos, 1.0);
}
)";

static const char* SOLID_FRAGMENT_SHADER = R"(
#version 450 core
in vec3 FragPos;
in vec3 Normal;
in vec4 Color;

out vec4 FragColor;

uniform vec3 uLightDir;
uniform vec3 uViewPos;

void main() {
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(uLightDir);

    // Ambient
    float ambient = 0.3;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Simple lighting
    vec3 lighting = vec3(ambient + diff * 0.7);
    vec3 finalColor = Color.rgb * lighting;

    FragColor = vec4(finalColor, Color.a);
}
)";

// Grid shader
static const char* GRID_VERTEX_SHADER = R"(
#version 450 core
layout(location = 0) in vec3 aPosition;
layout(location = 3) in vec4 aColor;

out vec4 Color;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    Color = aColor;
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
}
)";

static const char* GRID_FRAGMENT_SHADER = R"(
#version 450 core
in vec4 Color;
out vec4 FragColor;

void main() {
    FragColor = Color;
}
)";

AcceleratorRenderer::AcceleratorRenderer() = default;

AcceleratorRenderer::~AcceleratorRenderer() = default;

bool AcceleratorRenderer::init() {
    createShaders();
    createBaseMeshes();

    if (!m_solidShader.isValid() || !m_gridShader.isValid()) {
        PAS_ERROR("Failed to create accelerator shaders");
        return false;
    }

    return true;
}

void AcceleratorRenderer::createShaders() {
    m_solidShader.load(SOLID_VERTEX_SHADER, SOLID_FRAGMENT_SHADER);
    m_gridShader.load(GRID_VERTEX_SHADER, GRID_FRAGMENT_SHADER);
}

void AcceleratorRenderer::createBaseMeshes() {
    // Beam pipe - hollow tube
    m_tubeMesh = MeshFactory::createTube(0.04f, 0.05f, 1.0f, 24);
    m_tubeMesh.upload();

    // Magnet cylinder
    m_cylinderMesh = MeshFactory::createCylinder(0.1f, 1.0f, 24);
    m_cylinderMesh.upload();

    // Box for RF cavities
    m_boxMesh = MeshFactory::createCube(1.0f);
    m_boxMesh.upload();

    // Ground grid
    m_gridMesh = MeshFactory::createGrid(50.0f, 50.0f, 50, 50);
    m_gridMesh.upload();

    // Coordinate axes
    m_axesMesh = MeshFactory::createAxes(5.0f);
    m_axesMesh.upload();
}

void AcceleratorRenderer::buildGeometry(const accelerator::Accelerator& accelerator) {
    m_instances.clear();

    const auto& components = accelerator.getComponents();
    for (const auto& component : components) {
        addComponentGeometry(component);
    }
}

void AcceleratorRenderer::addComponentGeometry(const std::shared_ptr<accelerator::Component>& component) {
    glm::dvec3 pos = component->getPosition();

    switch (component->getType()) {
        case accelerator::ComponentType::BeamPipe: {
            auto* pipe = static_cast<accelerator::BeamPipe*>(component.get());
            buildBeamPipeGeometry(*pipe, pos);
            break;
        }
        case accelerator::ComponentType::Dipole: {
            auto* dipole = static_cast<accelerator::Dipole*>(component.get());
            buildDipoleGeometry(*dipole, pos);
            break;
        }
        case accelerator::ComponentType::Quadrupole: {
            auto* quad = static_cast<accelerator::Quadrupole*>(component.get());
            buildQuadrupoleGeometry(*quad, pos);
            break;
        }
        case accelerator::ComponentType::RFCavity: {
            auto* cavity = static_cast<accelerator::RFCavity*>(component.get());
            buildRFCavityGeometry(*cavity, pos);
            break;
        }
        default:
            break;
    }
}

void AcceleratorRenderer::buildBeamPipeGeometry(const accelerator::BeamPipe& pipe,
                                                 const glm::dvec3& position) {
    ComponentInstance instance;

    // Position along beam axis (Z) with length scaling
    float length = static_cast<float>(pipe.getLength());
    float sPos = static_cast<float>(pipe.getSPosition());

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, sPos + length * 0.5f));
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::scale(transform, glm::vec3(1.0f, length, 1.0f));

    instance.transform = transform;
    instance.color = glm::vec4(m_beamPipeColor, m_transparency);
    instance.type = accelerator::ComponentType::BeamPipe;
    instance.mesh = &m_tubeMesh;

    m_instances.push_back(instance);
}

void AcceleratorRenderer::buildDipoleGeometry(const accelerator::Dipole& dipole,
                                               const glm::dvec3& position) {
    ComponentInstance instance;

    float length = static_cast<float>(dipole.getLength());
    float sPos = static_cast<float>(dipole.getSPosition());

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, sPos + length * 0.5f));
    transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::scale(transform, glm::vec3(0.15f, length, 0.15f));

    instance.transform = transform;
    instance.color = glm::vec4(m_dipoleColor, 1.0f);
    instance.type = accelerator::ComponentType::Dipole;
    instance.mesh = &m_cylinderMesh;

    m_instances.push_back(instance);
}

void AcceleratorRenderer::buildQuadrupoleGeometry(const accelerator::Quadrupole& quad,
                                                   const glm::dvec3& position) {
    ComponentInstance instance;

    float length = static_cast<float>(quad.getLength());
    float sPos = static_cast<float>(quad.getSPosition());

    // Quadrupoles rendered as boxes to distinguish from dipoles
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, sPos + length * 0.5f));
    transform = glm::scale(transform, glm::vec3(0.2f, 0.2f, length));

    instance.transform = transform;

    // Color based on focusing/defocusing
    if (quad.isFocusing()) {
        instance.color = glm::vec4(m_quadrupoleColor, 1.0f);  // Red for focusing
    } else {
        instance.color = glm::vec4(0.3f, 0.3f, 0.9f, 1.0f);   // Blue for defocusing
    }

    instance.type = accelerator::ComponentType::Quadrupole;
    instance.mesh = &m_boxMesh;

    m_instances.push_back(instance);
}

void AcceleratorRenderer::buildRFCavityGeometry(const accelerator::RFCavity& cavity,
                                                 const glm::dvec3& position) {
    ComponentInstance instance;

    float length = static_cast<float>(cavity.getLength());
    float sPos = static_cast<float>(cavity.getSPosition());

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(0.0f, 0.0f, sPos + length * 0.5f));
    transform = glm::scale(transform, glm::vec3(0.25f, 0.25f, length));

    instance.transform = transform;
    instance.color = glm::vec4(m_rfCavityColor, 1.0f);
    instance.type = accelerator::ComponentType::RFCavity;
    instance.mesh = &m_boxMesh;

    m_instances.push_back(instance);
}

void AcceleratorRenderer::render(const glm::mat4& view, const glm::mat4& projection) {
    // Draw grid
    if (m_showGrid) {
        m_gridShader.use();
        m_gridShader.setMat4("uView", view);
        m_gridShader.setMat4("uProjection", projection);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_gridMesh.draw(GL_LINES);
        glDisable(GL_BLEND);
    }

    // Draw axes
    if (m_showAxes) {
        m_gridShader.use();
        m_gridShader.setMat4("uView", view);
        m_gridShader.setMat4("uProjection", projection);

        glLineWidth(2.0f);
        m_axesMesh.draw(GL_LINES);
        glLineWidth(1.0f);
    }

    // Draw components
    m_solidShader.use();
    m_solidShader.setMat4("uView", view);
    m_solidShader.setMat4("uProjection", projection);
    m_solidShader.setVec3("uLightDir", glm::vec3(0.5f, 1.0f, 0.3f));

    // Extract camera position from view matrix for specular
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 viewPos = glm::vec3(invView[3]);
    m_solidShader.setVec3("uViewPos", viewPos);

    // Enable blending for transparent components
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw opaque components first
    for (const auto& instance : m_instances) {
        if (instance.color.a >= 1.0f) {
            m_solidShader.setMat4("uModel", instance.transform);
            m_solidShader.setVec4("uColor", instance.color);
            instance.mesh->draw();
        }
    }

    // Then draw transparent components
    glDepthMask(GL_FALSE);
    for (const auto& instance : m_instances) {
        if (instance.color.a < 1.0f) {
            m_solidShader.setMat4("uModel", instance.transform);
            m_solidShader.setVec4("uColor", instance.color);
            instance.mesh->draw();
        }
    }
    glDepthMask(GL_TRUE);

    glDisable(GL_BLEND);
}

} // namespace pas::rendering
