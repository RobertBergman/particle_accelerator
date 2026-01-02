#include <gtest/gtest.h>
#include <cmath>

#include "rendering/Camera.hpp"

namespace pas::rendering::tests {

class CameraTest : public ::testing::Test {
protected:
    Camera camera;
    static constexpr float EPSILON = 1e-5f;
};

TEST_F(CameraTest, DefaultConstruction) {
    Camera cam;
    EXPECT_EQ(cam.getMode(), CameraMode::Orbit);
    EXPECT_FLOAT_EQ(cam.getFOV(), 45.0f);
}

TEST_F(CameraTest, AspectRatioConstruction) {
    Camera cam(16.0f / 9.0f);
    EXPECT_NEAR(cam.getAspectRatio(), 16.0f / 9.0f, EPSILON);
}

TEST_F(CameraTest, SetPosition) {
    camera.setPosition(glm::vec3(1.0f, 2.0f, 3.0f));

    EXPECT_FLOAT_EQ(camera.getPosition().x, 1.0f);
    EXPECT_FLOAT_EQ(camera.getPosition().y, 2.0f);
    EXPECT_FLOAT_EQ(camera.getPosition().z, 3.0f);
}

TEST_F(CameraTest, SetTarget) {
    camera.setTarget(glm::vec3(5.0f, 5.0f, 5.0f));

    EXPECT_FLOAT_EQ(camera.getTarget().x, 5.0f);
    EXPECT_FLOAT_EQ(camera.getTarget().y, 5.0f);
    EXPECT_FLOAT_EQ(camera.getTarget().z, 5.0f);
}

TEST_F(CameraTest, SetFOV) {
    camera.setFOV(60.0f);
    EXPECT_FLOAT_EQ(camera.getFOV(), 60.0f);

    // Test clamping
    camera.setFOV(0.0f);
    EXPECT_FLOAT_EQ(camera.getFOV(), 1.0f);

    camera.setFOV(200.0f);
    EXPECT_FLOAT_EQ(camera.getFOV(), 179.0f);
}

TEST_F(CameraTest, SetClipPlanes) {
    camera.setClipPlanes(0.5f, 500.0f);

    EXPECT_FLOAT_EQ(camera.getNearPlane(), 0.5f);
    EXPECT_FLOAT_EQ(camera.getFarPlane(), 500.0f);
}

TEST_F(CameraTest, InvalidClipPlanes) {
    float originalNear = camera.getNearPlane();
    float originalFar = camera.getFarPlane();

    // Near >= Far should not update
    camera.setClipPlanes(100.0f, 50.0f);

    EXPECT_FLOAT_EQ(camera.getNearPlane(), originalNear);
    EXPECT_FLOAT_EQ(camera.getFarPlane(), originalFar);

    // Negative near should not update
    camera.setClipPlanes(-1.0f, 100.0f);

    EXPECT_FLOAT_EQ(camera.getNearPlane(), originalNear);
}

TEST_F(CameraTest, CameraMode) {
    camera.setMode(CameraMode::Free);
    EXPECT_EQ(camera.getMode(), CameraMode::Free);

    camera.setMode(CameraMode::Orbit);
    EXPECT_EQ(camera.getMode(), CameraMode::Orbit);

    camera.setMode(CameraMode::Follow);
    EXPECT_EQ(camera.getMode(), CameraMode::Follow);
}

TEST_F(CameraTest, FreeModeMovement) {
    camera.setMode(CameraMode::Free);
    camera.setPosition(glm::vec3(0.0f));
    camera.setMoveSpeed(1.0f);

    glm::vec3 startPos = camera.getPosition();
    camera.moveForward(1.0f);
    camera.update();

    // Should have moved forward
    EXPECT_NE(camera.getPosition(), startPos);
}

TEST_F(CameraTest, OrbitMode) {
    camera.setMode(CameraMode::Orbit);
    camera.setTarget(glm::vec3(0.0f));
    camera.setOrbitDistance(10.0f);

    float distance = glm::length(camera.getPosition() - camera.getTarget());
    EXPECT_NEAR(distance, 10.0f, 0.5f);  // Allow some tolerance due to angle
}

TEST_F(CameraTest, OrbitZoom) {
    camera.setMode(CameraMode::Orbit);
    camera.setOrbitDistance(10.0f);
    camera.setTarget(glm::vec3(0.0f));

    camera.zoom(2.0f);  // Zoom in

    EXPECT_FLOAT_EQ(camera.getOrbitDistance(), 8.0f);
}

TEST_F(CameraTest, ViewMatrixUpdates) {
    camera.setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    camera.update();

    glm::mat4 view = camera.getViewMatrix();

    // View matrix should be valid (not identity for non-origin camera)
    EXPECT_NE(view, glm::mat4(1.0f));
}

TEST_F(CameraTest, ProjectionMatrixUpdates) {
    camera.setAspectRatio(2.0f);
    camera.setFOV(90.0f);
    camera.update();

    glm::mat4 proj = camera.getProjectionMatrix();

    // Projection matrix should be valid
    EXPECT_NE(proj, glm::mat4(1.0f));
}

TEST_F(CameraTest, ViewProjectionMatrix) {
    camera.update();

    glm::mat4 vp = camera.getViewProjectionMatrix();
    glm::mat4 expected = camera.getProjectionMatrix() * camera.getViewMatrix();

    // ViewProjection should equal Projection * View
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(vp[i][j], expected[i][j], EPSILON);
        }
    }
}

TEST_F(CameraTest, Reset) {
    camera.setPosition(glm::vec3(100.0f));
    camera.setFOV(120.0f);
    camera.setOrbitDistance(50.0f);

    camera.reset();

    EXPECT_FLOAT_EQ(camera.getFOV(), 45.0f);
    EXPECT_FLOAT_EQ(camera.getOrbitDistance(), 15.0f);
}

TEST_F(CameraTest, DirectionVectors) {
    camera.update();

    glm::vec3 forward = camera.getForward();
    glm::vec3 right = camera.getRight();
    glm::vec3 up = camera.getUp();

    // Direction vectors should be normalized
    EXPECT_NEAR(glm::length(forward), 1.0f, EPSILON);
    EXPECT_NEAR(glm::length(right), 1.0f, EPSILON);
    EXPECT_NEAR(glm::length(up), 1.0f, EPSILON);

    // Vectors should be orthogonal
    EXPECT_NEAR(glm::dot(forward, right), 0.0f, EPSILON);
    EXPECT_NEAR(glm::dot(forward, up), 0.0f, EPSILON);
    EXPECT_NEAR(glm::dot(right, up), 0.0f, EPSILON);
}

TEST_F(CameraTest, RotationAffectsForward) {
    camera.setMode(CameraMode::Free);
    camera.setRotationSpeed(1.0f);  // Set rotation speed to 1.0 so input degrees = actual degrees

    glm::vec3 originalForward = camera.getForward();

    camera.rotate(90.0f, 0.0f);  // 90 degree yaw
    camera.update();

    glm::vec3 newForward = camera.getForward();

    // Forward should have changed significantly
    float dotProduct = glm::dot(originalForward, newForward);
    EXPECT_LT(dotProduct, 0.5f);  // Should be roughly perpendicular
}

TEST_F(CameraTest, PitchClamping) {
    camera.rotate(0.0f, 1000.0f);  // Large pitch rotation

    // Pitch should be clamped
    EXPECT_LE(camera.getPitch(), 89.0f);
    EXPECT_GE(camera.getPitch(), -89.0f);
}

} // namespace pas::rendering::tests
