#pragma once

#include <glm/glm.hpp>

namespace pas::rendering {

/**
 * @brief Camera mode.
 */
enum class CameraMode {
    Free,    // WASD flight camera
    Orbit,   // Orbit around target point
    Follow   // Follow beam centroid
};

/**
 * @brief 3D camera for scene viewing.
 *
 * Manages view and projection matrices with multiple camera modes.
 */
class Camera {
public:
    Camera();
    explicit Camera(float aspectRatio);

    /**
     * @brief Update camera matrices.
     */
    void update();

    // Matrix getters
    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
    glm::mat4 getViewProjectionMatrix() const { return m_projectionMatrix * m_viewMatrix; }

    // Position and orientation
    const glm::vec3& getPosition() const { return m_position; }
    void setPosition(const glm::vec3& position);

    const glm::vec3& getTarget() const { return m_target; }
    void setTarget(const glm::vec3& target);

    glm::vec3 getForward() const { return m_front; }
    glm::vec3 getRight() const { return m_right; }
    glm::vec3 getUp() const { return m_up; }

    // Camera mode
    CameraMode getMode() const { return m_mode; }
    void setMode(CameraMode mode);

    // Perspective settings
    float getFOV() const { return m_fov; }
    void setFOV(float fov);

    float getAspectRatio() const { return m_aspectRatio; }
    void setAspectRatio(float ratio);

    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }
    void setClipPlanes(float nearPlane, float farPlane);

    // Movement (for Free mode)
    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);

    // Rotation
    void rotate(float yawDelta, float pitchDelta);
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }

    // Orbit mode
    float getOrbitDistance() const { return m_orbitDistance; }
    void setOrbitDistance(float distance);
    void orbit(float yawDelta, float pitchDelta);
    void zoom(float amount);

    // Speed settings
    float getMoveSpeed() const { return m_moveSpeed; }
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }

    float getRotationSpeed() const { return m_rotationSpeed; }
    void setRotationSpeed(float speed) { m_rotationSpeed = speed; }

    // Reset camera
    void reset();

    // Utility
    glm::vec3 screenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize, float depth = 1.0f) const;

private:
    void updateVectors();
    void updateViewMatrix();
    void updateProjectionMatrix();

    // Matrices
    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projectionMatrix{1.0f};

    // Position and orientation
    glm::vec3 m_position{0.0f, 5.0f, 10.0f};
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    // Euler angles
    float m_yaw = -90.0f;    // Yaw in degrees
    float m_pitch = -20.0f;  // Pitch in degrees

    // Perspective
    float m_fov = 45.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    // Mode
    CameraMode m_mode = CameraMode::Orbit;
    float m_orbitDistance = 15.0f;

    // Speed settings
    float m_moveSpeed = 10.0f;
    float m_rotationSpeed = 0.1f;

    // Dirty flags
    bool m_viewDirty = true;
    bool m_projectionDirty = true;
};

} // namespace pas::rendering
