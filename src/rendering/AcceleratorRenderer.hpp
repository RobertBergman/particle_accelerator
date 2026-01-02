#pragma once

#include "rendering/Shader.hpp"
#include "rendering/Mesh.hpp"
#include "accelerator/Accelerator.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace pas::rendering {

/**
 * @brief Renderer for accelerator hardware visualization.
 *
 * Renders the static beamline components with color-coded magnets
 * and optional cutaway views.
 */
class AcceleratorRenderer {
public:
    AcceleratorRenderer();
    ~AcceleratorRenderer();

    /**
     * @brief Initialize rendering resources.
     * @return True if initialization succeeded.
     */
    bool init();

    /**
     * @brief Build renderable geometry from accelerator.
     * @param accelerator The accelerator to visualize.
     */
    void buildGeometry(const accelerator::Accelerator& accelerator);

    /**
     * @brief Render the accelerator.
     * @param view View matrix.
     * @param projection Projection matrix.
     */
    void render(const glm::mat4& view, const glm::mat4& projection);

    // Visualization settings
    void setCutawayEnabled(bool enabled) { m_cutawayEnabled = enabled; }
    bool isCutawayEnabled() const { return m_cutawayEnabled; }

    void setTransparency(float alpha) { m_transparency = alpha; }
    float getTransparency() const { return m_transparency; }

    void setDipoleColor(const glm::vec3& color) { m_dipoleColor = color; }
    void setQuadrupoleColor(const glm::vec3& color) { m_quadrupoleColor = color; }
    void setRFCavityColor(const glm::vec3& color) { m_rfCavityColor = color; }
    void setBeamPipeColor(const glm::vec3& color) { m_beamPipeColor = color; }

    void setShowGrid(bool show) { m_showGrid = show; }
    bool getShowGrid() const { return m_showGrid; }

    void setShowAxes(bool show) { m_showAxes = show; }
    bool getShowAxes() const { return m_showAxes; }

private:
    void createShaders();
    void createBaseMeshes();
    void addComponentGeometry(const std::shared_ptr<accelerator::Component>& component);
    void buildBeamPipeGeometry(const accelerator::BeamPipe& pipe, const glm::dvec3& position);
    void buildDipoleGeometry(const accelerator::Dipole& dipole, const glm::dvec3& position);
    void buildQuadrupoleGeometry(const accelerator::Quadrupole& quad, const glm::dvec3& position);
    void buildRFCavityGeometry(const accelerator::RFCavity& cavity, const glm::dvec3& position);

    struct ComponentInstance {
        glm::mat4 transform;
        glm::vec4 color;
        accelerator::ComponentType type;
        Mesh* mesh;
    };

    Shader m_solidShader;
    Shader m_gridShader;

    // Base meshes for component types
    Mesh m_tubeMesh;
    Mesh m_cylinderMesh;
    Mesh m_boxMesh;
    Mesh m_gridMesh;
    Mesh m_axesMesh;

    // Component instances to render
    std::vector<ComponentInstance> m_instances;

    // Visualization settings
    bool m_cutawayEnabled = true;
    float m_transparency = 0.7f;

    glm::vec3 m_dipoleColor{0.3f, 0.5f, 0.9f};      // Blue
    glm::vec3 m_quadrupoleColor{0.9f, 0.3f, 0.3f};  // Red
    glm::vec3 m_rfCavityColor{0.3f, 0.9f, 0.3f};    // Green
    glm::vec3 m_beamPipeColor{0.6f, 0.6f, 0.6f};    // Gray

    bool m_showGrid = true;
    bool m_showAxes = true;
};

} // namespace pas::rendering
