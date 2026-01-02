#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace pas::rendering {

/**
 * @brief Vertex data structure.
 */
struct Vertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 texCoords{0.0f};
    glm::vec4 color{1.0f};
};

/**
 * @brief Mesh class for OpenGL geometry.
 *
 * Manages vertex buffers, index buffers, and VAO for rendering.
 */
class Mesh {
public:
    Mesh();
    ~Mesh();

    // Non-copyable
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Movable
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    /**
     * @brief Set mesh data.
     * @param vertices Vertex data.
     * @param indices Index data.
     */
    void setData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Set vertex data only (for point rendering).
     */
    void setVertices(const std::vector<Vertex>& vertices);

    /**
     * @brief Upload data to GPU buffers.
     */
    void upload();

    /**
     * @brief Bind the VAO for rendering.
     */
    void bind() const;

    /**
     * @brief Unbind the VAO.
     */
    void unbind() const;

    /**
     * @brief Draw the mesh.
     * @param mode OpenGL primitive mode (e.g., GL_TRIANGLES).
     */
    void draw(unsigned int mode = 0x0004) const;  // GL_TRIANGLES

    /**
     * @brief Draw as points.
     */
    void drawPoints() const;

    /**
     * @brief Check if mesh has data.
     */
    bool hasData() const { return !m_vertices.empty(); }

    /**
     * @brief Get vertex count.
     */
    size_t getVertexCount() const { return m_vertices.size(); }

    /**
     * @brief Get index count.
     */
    size_t getIndexCount() const { return m_indices.size(); }

    /**
     * @brief Check if mesh uses indices.
     */
    bool hasIndices() const { return !m_indices.empty(); }

    // Access to raw data
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<unsigned int>& getIndices() const { return m_indices; }

private:
    void cleanup();
    void setupBuffers();

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
    bool m_uploaded = false;
};

/**
 * @brief Factory for creating primitive meshes.
 */
class MeshFactory {
public:
    /**
     * @brief Create a unit cube centered at origin.
     */
    static Mesh createCube(float size = 1.0f);

    /**
     * @brief Create a UV sphere.
     * @param radius Sphere radius.
     * @param segments Number of longitude segments.
     * @param rings Number of latitude rings.
     */
    static Mesh createSphere(float radius = 1.0f, int segments = 32, int rings = 16);

    /**
     * @brief Create a cylinder along Y axis.
     * @param radius Cylinder radius.
     * @param height Cylinder height.
     * @param segments Number of segments around circumference.
     */
    static Mesh createCylinder(float radius = 1.0f, float height = 1.0f, int segments = 32);

    /**
     * @brief Create a torus (donut shape).
     * @param majorRadius Distance from center to tube center.
     * @param minorRadius Tube radius.
     * @param majorSegments Segments around the ring.
     * @param minorSegments Segments around the tube.
     */
    static Mesh createTorus(float majorRadius = 1.0f, float minorRadius = 0.3f,
                            int majorSegments = 32, int minorSegments = 16);

    /**
     * @brief Create a flat grid/plane.
     * @param width Width of the grid.
     * @param height Height of the grid.
     * @param divisionsX Subdivisions along X.
     * @param divisionsZ Subdivisions along Z.
     */
    static Mesh createGrid(float width = 10.0f, float height = 10.0f,
                           int divisionsX = 10, int divisionsZ = 10);

    /**
     * @brief Create a tube/pipe (hollow cylinder).
     * @param innerRadius Inner radius.
     * @param outerRadius Outer radius.
     * @param length Tube length.
     * @param segments Number of segments.
     */
    static Mesh createTube(float innerRadius = 0.8f, float outerRadius = 1.0f,
                           float length = 2.0f, int segments = 32);

    /**
     * @brief Create coordinate axes visualization.
     * @param length Length of each axis.
     */
    static Mesh createAxes(float length = 1.0f);
};

} // namespace pas::rendering
