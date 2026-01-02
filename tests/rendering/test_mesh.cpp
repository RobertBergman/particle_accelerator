#include <gtest/gtest.h>
#include <cmath>

#include "rendering/Mesh.hpp"

// Note: These tests only test data creation, not GPU operations
// GPU operations (upload, bind, draw) require an OpenGL context

namespace pas::rendering::tests {

class MeshTest : public ::testing::Test {
protected:
    static constexpr float EPSILON = 1e-5f;
};

TEST_F(MeshTest, DefaultConstruction) {
    Mesh mesh;
    EXPECT_FALSE(mesh.hasData());
    EXPECT_EQ(mesh.getVertexCount(), 0u);
    EXPECT_EQ(mesh.getIndexCount(), 0u);
}

TEST_F(MeshTest, SetDataWithIndices) {
    std::vector<Vertex> vertices = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}, {1, 1, 1, 1}},
        {{1, 0, 0}, {0, 1, 0}, {1, 0}, {1, 1, 1, 1}},
        {{0, 0, 1}, {0, 1, 0}, {0, 1}, {1, 1, 1, 1}},
    };
    std::vector<unsigned int> indices = {0, 1, 2};

    Mesh mesh;
    mesh.setData(vertices, indices);

    EXPECT_TRUE(mesh.hasData());
    EXPECT_EQ(mesh.getVertexCount(), 3u);
    EXPECT_EQ(mesh.getIndexCount(), 3u);
    EXPECT_TRUE(mesh.hasIndices());
}

TEST_F(MeshTest, SetVerticesOnly) {
    std::vector<Vertex> vertices = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}, {1, 1, 1, 1}},
        {{1, 0, 0}, {0, 1, 0}, {1, 0}, {1, 1, 1, 1}},
    };

    Mesh mesh;
    mesh.setVertices(vertices);

    EXPECT_TRUE(mesh.hasData());
    EXPECT_EQ(mesh.getVertexCount(), 2u);
    EXPECT_FALSE(mesh.hasIndices());
}

TEST_F(MeshTest, MoveConstruction) {
    std::vector<Vertex> vertices = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}, {1, 1, 1, 1}},
    };

    Mesh mesh1;
    mesh1.setVertices(vertices);

    Mesh mesh2(std::move(mesh1));

    EXPECT_TRUE(mesh2.hasData());
    EXPECT_EQ(mesh2.getVertexCount(), 1u);
}

TEST_F(MeshTest, MoveAssignment) {
    std::vector<Vertex> vertices = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}, {1, 1, 1, 1}},
    };

    Mesh mesh1;
    mesh1.setVertices(vertices);

    Mesh mesh2;
    mesh2 = std::move(mesh1);

    EXPECT_TRUE(mesh2.hasData());
    EXPECT_EQ(mesh2.getVertexCount(), 1u);
}

// MeshFactory tests

TEST_F(MeshTest, CreateCube) {
    Mesh cube = MeshFactory::createCube(2.0f);

    EXPECT_TRUE(cube.hasData());
    EXPECT_EQ(cube.getVertexCount(), 24u);  // 6 faces * 4 vertices
    EXPECT_EQ(cube.getIndexCount(), 36u);   // 6 faces * 2 triangles * 3
}

TEST_F(MeshTest, CreateCubeVertexPositions) {
    Mesh cube = MeshFactory::createCube(2.0f);
    const auto& vertices = cube.getVertices();

    // Check that all vertices are within bounds
    for (const auto& v : vertices) {
        EXPECT_LE(std::abs(v.position.x), 1.0f + EPSILON);
        EXPECT_LE(std::abs(v.position.y), 1.0f + EPSILON);
        EXPECT_LE(std::abs(v.position.z), 1.0f + EPSILON);
    }
}

TEST_F(MeshTest, CreateSphere) {
    Mesh sphere = MeshFactory::createSphere(1.0f, 16, 8);

    EXPECT_TRUE(sphere.hasData());
    EXPECT_GT(sphere.getVertexCount(), 0u);
    EXPECT_GT(sphere.getIndexCount(), 0u);
}

TEST_F(MeshTest, CreateSphereRadius) {
    float radius = 2.5f;
    Mesh sphere = MeshFactory::createSphere(radius, 16, 8);
    const auto& vertices = sphere.getVertices();

    // All vertices should be at radius distance from origin
    for (const auto& v : vertices) {
        float dist = glm::length(v.position);
        EXPECT_NEAR(dist, radius, EPSILON);
    }
}

TEST_F(MeshTest, CreateCylinder) {
    Mesh cylinder = MeshFactory::createCylinder(1.0f, 2.0f, 16);

    EXPECT_TRUE(cylinder.hasData());
    EXPECT_GT(cylinder.getVertexCount(), 0u);
    EXPECT_GT(cylinder.getIndexCount(), 0u);
}

TEST_F(MeshTest, CreateTorus) {
    Mesh torus = MeshFactory::createTorus(2.0f, 0.5f, 16, 8);

    EXPECT_TRUE(torus.hasData());
    EXPECT_GT(torus.getVertexCount(), 0u);
    EXPECT_GT(torus.getIndexCount(), 0u);
}

TEST_F(MeshTest, CreateGrid) {
    Mesh grid = MeshFactory::createGrid(10.0f, 10.0f, 5, 5);

    EXPECT_TRUE(grid.hasData());

    // Grid should have (divisions+1)^2 vertices
    EXPECT_EQ(grid.getVertexCount(), 36u);  // 6 * 6 = 36
}

TEST_F(MeshTest, CreateTube) {
    Mesh tube = MeshFactory::createTube(0.8f, 1.0f, 2.0f, 16);

    EXPECT_TRUE(tube.hasData());
    EXPECT_GT(tube.getVertexCount(), 0u);
    EXPECT_GT(tube.getIndexCount(), 0u);
}

TEST_F(MeshTest, CreateAxes) {
    Mesh axes = MeshFactory::createAxes(5.0f);

    EXPECT_TRUE(axes.hasData());
    EXPECT_EQ(axes.getVertexCount(), 6u);  // 2 vertices per axis * 3 axes
    EXPECT_FALSE(axes.hasIndices());  // Axes use vertex-only rendering
}

TEST_F(MeshTest, CreateAxesColors) {
    Mesh axes = MeshFactory::createAxes(1.0f);
    const auto& vertices = axes.getVertices();

    // X axis should be red
    EXPECT_NEAR(vertices[0].color.r, 1.0f, EPSILON);
    EXPECT_NEAR(vertices[0].color.g, 0.0f, EPSILON);
    EXPECT_NEAR(vertices[0].color.b, 0.0f, EPSILON);

    // Y axis should be green
    EXPECT_NEAR(vertices[2].color.r, 0.0f, EPSILON);
    EXPECT_NEAR(vertices[2].color.g, 1.0f, EPSILON);
    EXPECT_NEAR(vertices[2].color.b, 0.0f, EPSILON);

    // Z axis should be blue
    EXPECT_NEAR(vertices[4].color.r, 0.0f, EPSILON);
    EXPECT_NEAR(vertices[4].color.g, 0.0f, EPSILON);
    EXPECT_NEAR(vertices[4].color.b, 1.0f, EPSILON);
}

TEST_F(MeshTest, NormalsAreNormalized) {
    Mesh sphere = MeshFactory::createSphere(1.0f, 8, 4);
    const auto& vertices = sphere.getVertices();

    for (const auto& v : vertices) {
        float len = glm::length(v.normal);
        EXPECT_NEAR(len, 1.0f, EPSILON);
    }
}

TEST_F(MeshTest, TexCoordsInRange) {
    Mesh sphere = MeshFactory::createSphere(1.0f, 8, 4);
    const auto& vertices = sphere.getVertices();

    for (const auto& v : vertices) {
        EXPECT_GE(v.texCoords.x, 0.0f - EPSILON);
        EXPECT_LE(v.texCoords.x, 1.0f + EPSILON);
        EXPECT_GE(v.texCoords.y, 0.0f - EPSILON);
        EXPECT_LE(v.texCoords.y, 1.0f + EPSILON);
    }
}

TEST_F(MeshTest, IndicesInRange) {
    Mesh cube = MeshFactory::createCube(1.0f);
    const auto& indices = cube.getIndices();
    size_t vertexCount = cube.getVertexCount();

    for (unsigned int idx : indices) {
        EXPECT_LT(idx, vertexCount);
    }
}

} // namespace pas::rendering::tests
