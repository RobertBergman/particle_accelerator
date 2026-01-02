#include "rendering/Mesh.hpp"

#include <glad/glad.h>
#include <cmath>
#include <numbers>

namespace pas::rendering {

Mesh::Mesh() = default;

Mesh::~Mesh() {
    cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vertices(std::move(other.m_vertices))
    , m_indices(std::move(other.m_indices))
    , m_vao(other.m_vao)
    , m_vbo(other.m_vbo)
    , m_ebo(other.m_ebo)
    , m_uploaded(other.m_uploaded)
{
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
    other.m_uploaded = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ebo = other.m_ebo;
        m_uploaded = other.m_uploaded;
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
        other.m_uploaded = false;
    }
    return *this;
}

void Mesh::setData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    m_vertices = vertices;
    m_indices = indices;
    m_uploaded = false;
}

void Mesh::setVertices(const std::vector<Vertex>& vertices) {
    m_vertices = vertices;
    m_indices.clear();
    m_uploaded = false;
}

void Mesh::upload() {
    if (m_uploaded || m_vertices.empty()) return;

    cleanup();
    setupBuffers();
    m_uploaded = true;
}

void Mesh::bind() const {
    if (m_vao != 0) {
        glBindVertexArray(m_vao);
    }
}

void Mesh::unbind() const {
    glBindVertexArray(0);
}

void Mesh::draw(unsigned int mode) const {
    if (!m_uploaded || m_vao == 0) return;

    bind();
    if (!m_indices.empty()) {
        glDrawElements(mode, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(mode, 0, static_cast<GLsizei>(m_vertices.size()));
    }
}

void Mesh::drawPoints() const {
    draw(GL_POINTS);
}

void Mesh::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
    m_uploaded = false;
}

void Mesh::setupBuffers() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)),
                 m_vertices.data(),
                 GL_STATIC_DRAW);

    // Upload index data if present
    if (!m_indices.empty()) {
        glGenBuffers(1, &m_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(m_indices.size() * sizeof(unsigned int)),
                     m_indices.data(),
                     GL_STATIC_DRAW);
    }

    // Vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    // TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, texCoords)));

    // Color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, color)));

    glBindVertexArray(0);
}

// MeshFactory implementations

Mesh MeshFactory::createCube(float size) {
    float h = size * 0.5f;

    std::vector<Vertex> vertices = {
        // Front face
        {{-h, -h,  h}, { 0,  0,  1}, {0, 0}, {1, 1, 1, 1}},
        {{ h, -h,  h}, { 0,  0,  1}, {1, 0}, {1, 1, 1, 1}},
        {{ h,  h,  h}, { 0,  0,  1}, {1, 1}, {1, 1, 1, 1}},
        {{-h,  h,  h}, { 0,  0,  1}, {0, 1}, {1, 1, 1, 1}},
        // Back face
        {{ h, -h, -h}, { 0,  0, -1}, {0, 0}, {1, 1, 1, 1}},
        {{-h, -h, -h}, { 0,  0, -1}, {1, 0}, {1, 1, 1, 1}},
        {{-h,  h, -h}, { 0,  0, -1}, {1, 1}, {1, 1, 1, 1}},
        {{ h,  h, -h}, { 0,  0, -1}, {0, 1}, {1, 1, 1, 1}},
        // Top face
        {{-h,  h,  h}, { 0,  1,  0}, {0, 0}, {1, 1, 1, 1}},
        {{ h,  h,  h}, { 0,  1,  0}, {1, 0}, {1, 1, 1, 1}},
        {{ h,  h, -h}, { 0,  1,  0}, {1, 1}, {1, 1, 1, 1}},
        {{-h,  h, -h}, { 0,  1,  0}, {0, 1}, {1, 1, 1, 1}},
        // Bottom face
        {{-h, -h, -h}, { 0, -1,  0}, {0, 0}, {1, 1, 1, 1}},
        {{ h, -h, -h}, { 0, -1,  0}, {1, 0}, {1, 1, 1, 1}},
        {{ h, -h,  h}, { 0, -1,  0}, {1, 1}, {1, 1, 1, 1}},
        {{-h, -h,  h}, { 0, -1,  0}, {0, 1}, {1, 1, 1, 1}},
        // Right face
        {{ h, -h,  h}, { 1,  0,  0}, {0, 0}, {1, 1, 1, 1}},
        {{ h, -h, -h}, { 1,  0,  0}, {1, 0}, {1, 1, 1, 1}},
        {{ h,  h, -h}, { 1,  0,  0}, {1, 1}, {1, 1, 1, 1}},
        {{ h,  h,  h}, { 1,  0,  0}, {0, 1}, {1, 1, 1, 1}},
        // Left face
        {{-h, -h, -h}, {-1,  0,  0}, {0, 0}, {1, 1, 1, 1}},
        {{-h, -h,  h}, {-1,  0,  0}, {1, 0}, {1, 1, 1, 1}},
        {{-h,  h,  h}, {-1,  0,  0}, {1, 1}, {1, 1, 1, 1}},
        {{-h,  h, -h}, {-1,  0,  0}, {0, 1}, {1, 1, 1, 1}},
    };

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < 6; ++i) {
        unsigned int base = i * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createSphere(float radius, int segments, int rings) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    constexpr float pi = std::numbers::pi_v<float>;

    for (int y = 0; y <= rings; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(rings);
            float xPos = std::cos(xSegment * 2.0f * pi) * std::sin(ySegment * pi);
            float yPos = std::cos(ySegment * pi);
            float zPos = std::sin(xSegment * 2.0f * pi) * std::sin(ySegment * pi);

            Vertex v;
            v.position = glm::vec3(xPos, yPos, zPos) * radius;
            v.normal = glm::vec3(xPos, yPos, zPos);
            v.texCoords = glm::vec2(xSegment, ySegment);
            v.color = glm::vec4(1.0f);
            vertices.push_back(v);
        }
    }

    for (int y = 0; y < rings; ++y) {
        for (int x = 0; x < segments; ++x) {
            unsigned int i0 = y * (segments + 1) + x;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = (y + 1) * (segments + 1) + x;
            unsigned int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createCylinder(float radius, float height, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    constexpr float pi = std::numbers::pi_v<float>;
    float halfHeight = height * 0.5f;

    // Side vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * pi;
        float x = std::cos(angle);
        float z = std::sin(angle);

        // Bottom vertex
        Vertex vBottom;
        vBottom.position = glm::vec3(x * radius, -halfHeight, z * radius);
        vBottom.normal = glm::vec3(x, 0, z);
        vBottom.texCoords = glm::vec2(static_cast<float>(i) / segments, 0.0f);
        vBottom.color = glm::vec4(1.0f);
        vertices.push_back(vBottom);

        // Top vertex
        Vertex vTop;
        vTop.position = glm::vec3(x * radius, halfHeight, z * radius);
        vTop.normal = glm::vec3(x, 0, z);
        vTop.texCoords = glm::vec2(static_cast<float>(i) / segments, 1.0f);
        vTop.color = glm::vec4(1.0f);
        vertices.push_back(vTop);
    }

    // Side indices
    for (int i = 0; i < segments; ++i) {
        unsigned int i0 = i * 2;
        unsigned int i1 = i0 + 1;
        unsigned int i2 = i0 + 2;
        unsigned int i3 = i0 + 3;

        indices.push_back(i0);
        indices.push_back(i2);
        indices.push_back(i1);
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i3);
    }

    // Top cap
    unsigned int topCenterIndex = static_cast<unsigned int>(vertices.size());
    Vertex topCenter;
    topCenter.position = glm::vec3(0, halfHeight, 0);
    topCenter.normal = glm::vec3(0, 1, 0);
    topCenter.texCoords = glm::vec2(0.5f, 0.5f);
    topCenter.color = glm::vec4(1.0f);
    vertices.push_back(topCenter);

    for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * pi;
        Vertex v;
        v.position = glm::vec3(std::cos(angle) * radius, halfHeight, std::sin(angle) * radius);
        v.normal = glm::vec3(0, 1, 0);
        v.texCoords = glm::vec2(std::cos(angle) * 0.5f + 0.5f, std::sin(angle) * 0.5f + 0.5f);
        v.color = glm::vec4(1.0f);
        vertices.push_back(v);
    }

    for (int i = 0; i < segments; ++i) {
        indices.push_back(topCenterIndex);
        indices.push_back(topCenterIndex + 1 + i);
        indices.push_back(topCenterIndex + 2 + i);
    }

    // Bottom cap
    unsigned int bottomCenterIndex = static_cast<unsigned int>(vertices.size());
    Vertex bottomCenter;
    bottomCenter.position = glm::vec3(0, -halfHeight, 0);
    bottomCenter.normal = glm::vec3(0, -1, 0);
    bottomCenter.texCoords = glm::vec2(0.5f, 0.5f);
    bottomCenter.color = glm::vec4(1.0f);
    vertices.push_back(bottomCenter);

    for (int i = 0; i <= segments; ++i) {
        float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * pi;
        Vertex v;
        v.position = glm::vec3(std::cos(angle) * radius, -halfHeight, std::sin(angle) * radius);
        v.normal = glm::vec3(0, -1, 0);
        v.texCoords = glm::vec2(std::cos(angle) * 0.5f + 0.5f, std::sin(angle) * 0.5f + 0.5f);
        v.color = glm::vec4(1.0f);
        vertices.push_back(v);
    }

    for (int i = 0; i < segments; ++i) {
        indices.push_back(bottomCenterIndex);
        indices.push_back(bottomCenterIndex + 2 + i);
        indices.push_back(bottomCenterIndex + 1 + i);
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createTorus(float majorRadius, float minorRadius,
                               int majorSegments, int minorSegments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    constexpr float pi = std::numbers::pi_v<float>;

    for (int i = 0; i <= majorSegments; ++i) {
        float u = static_cast<float>(i) / static_cast<float>(majorSegments) * 2.0f * pi;

        for (int j = 0; j <= minorSegments; ++j) {
            float v = static_cast<float>(j) / static_cast<float>(minorSegments) * 2.0f * pi;

            float x = (majorRadius + minorRadius * std::cos(v)) * std::cos(u);
            float y = minorRadius * std::sin(v);
            float z = (majorRadius + minorRadius * std::cos(v)) * std::sin(u);

            float nx = std::cos(v) * std::cos(u);
            float ny = std::sin(v);
            float nz = std::cos(v) * std::sin(u);

            Vertex vert;
            vert.position = glm::vec3(x, y, z);
            vert.normal = glm::vec3(nx, ny, nz);
            vert.texCoords = glm::vec2(static_cast<float>(i) / majorSegments,
                                       static_cast<float>(j) / minorSegments);
            vert.color = glm::vec4(1.0f);
            vertices.push_back(vert);
        }
    }

    for (int i = 0; i < majorSegments; ++i) {
        for (int j = 0; j < minorSegments; ++j) {
            unsigned int i0 = i * (minorSegments + 1) + j;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = (i + 1) * (minorSegments + 1) + j;
            unsigned int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createGrid(float width, float height, int divisionsX, int divisionsZ) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float stepX = width / static_cast<float>(divisionsX);
    float stepZ = height / static_cast<float>(divisionsZ);
    float startX = -width * 0.5f;
    float startZ = -height * 0.5f;

    for (int z = 0; z <= divisionsZ; ++z) {
        for (int x = 0; x <= divisionsX; ++x) {
            Vertex v;
            v.position = glm::vec3(startX + x * stepX, 0.0f, startZ + z * stepZ);
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            v.texCoords = glm::vec2(static_cast<float>(x) / divisionsX,
                                    static_cast<float>(z) / divisionsZ);
            v.color = glm::vec4(1.0f);
            vertices.push_back(v);
        }
    }

    for (int z = 0; z < divisionsZ; ++z) {
        for (int x = 0; x < divisionsX; ++x) {
            unsigned int i0 = z * (divisionsX + 1) + x;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = (z + 1) * (divisionsX + 1) + x;
            unsigned int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createTube(float innerRadius, float outerRadius, float length, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    constexpr float pi = std::numbers::pi_v<float>;
    float halfLength = length * 0.5f;

    // Create outer and inner surfaces
    for (int surface = 0; surface < 2; ++surface) {
        float radius = (surface == 0) ? outerRadius : innerRadius;
        float normalSign = (surface == 0) ? 1.0f : -1.0f;

        for (int i = 0; i <= segments; ++i) {
            float angle = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * pi;
            float x = std::cos(angle);
            float z = std::sin(angle);

            // Front vertex
            Vertex vFront;
            vFront.position = glm::vec3(x * radius, -halfLength, z * radius);
            vFront.normal = glm::vec3(x * normalSign, 0, z * normalSign);
            vFront.texCoords = glm::vec2(static_cast<float>(i) / segments, 0.0f);
            vFront.color = glm::vec4(1.0f);
            vertices.push_back(vFront);

            // Back vertex
            Vertex vBack;
            vBack.position = glm::vec3(x * radius, halfLength, z * radius);
            vBack.normal = glm::vec3(x * normalSign, 0, z * normalSign);
            vBack.texCoords = glm::vec2(static_cast<float>(i) / segments, 1.0f);
            vBack.color = glm::vec4(1.0f);
            vertices.push_back(vBack);
        }
    }

    // Outer surface indices
    for (int i = 0; i < segments; ++i) {
        unsigned int i0 = i * 2;
        unsigned int i1 = i0 + 1;
        unsigned int i2 = i0 + 2;
        unsigned int i3 = i0 + 3;

        indices.push_back(i0);
        indices.push_back(i2);
        indices.push_back(i1);
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i3);
    }

    // Inner surface indices (reversed winding)
    unsigned int innerOffset = (segments + 1) * 2;
    for (int i = 0; i < segments; ++i) {
        unsigned int i0 = innerOffset + i * 2;
        unsigned int i1 = i0 + 1;
        unsigned int i2 = i0 + 2;
        unsigned int i3 = i0 + 3;

        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
        indices.push_back(i1);
        indices.push_back(i3);
        indices.push_back(i2);
    }

    Mesh mesh;
    mesh.setData(vertices, indices);
    return mesh;
}

Mesh MeshFactory::createAxes(float length) {
    std::vector<Vertex> vertices;

    // X axis - Red
    vertices.push_back({{0, 0, 0}, {1, 0, 0}, {0, 0}, {1, 0, 0, 1}});
    vertices.push_back({{length, 0, 0}, {1, 0, 0}, {1, 0}, {1, 0, 0, 1}});

    // Y axis - Green
    vertices.push_back({{0, 0, 0}, {0, 1, 0}, {0, 0}, {0, 1, 0, 1}});
    vertices.push_back({{0, length, 0}, {0, 1, 0}, {1, 0}, {0, 1, 0, 1}});

    // Z axis - Blue
    vertices.push_back({{0, 0, 0}, {0, 0, 1}, {0, 0}, {0, 0, 1, 1}});
    vertices.push_back({{0, 0, length}, {0, 0, 1}, {1, 0}, {0, 0, 1, 1}});

    Mesh mesh;
    mesh.setVertices(vertices);
    return mesh;
}

} // namespace pas::rendering
