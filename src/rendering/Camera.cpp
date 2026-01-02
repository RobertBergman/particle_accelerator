#include "rendering/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace pas::rendering {

Camera::Camera() {
    updateVectors();
    updateViewMatrix();
    updateProjectionMatrix();
}

Camera::Camera(float aspectRatio)
    : m_aspectRatio(aspectRatio) {
    updateVectors();
    updateViewMatrix();
    updateProjectionMatrix();
}

void Camera::update() {
    if (m_viewDirty) {
        updateViewMatrix();
        m_viewDirty = false;
    }
    if (m_projectionDirty) {
        updateProjectionMatrix();
        m_projectionDirty = false;
    }
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
    m_viewDirty = true;
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    if (m_mode == CameraMode::Orbit) {
        // Recalculate position based on new target
        m_position = m_target - m_front * m_orbitDistance;
    }
    m_viewDirty = true;
}

void Camera::setMode(CameraMode mode) {
    if (m_mode != mode) {
        m_mode = mode;
        if (mode == CameraMode::Orbit) {
            // Calculate orbit distance from current position
            m_orbitDistance = glm::length(m_position - m_target);
            if (m_orbitDistance < 1.0f) m_orbitDistance = 15.0f;
        }
        m_viewDirty = true;
    }
}

void Camera::setFOV(float fov) {
    m_fov = std::clamp(fov, 1.0f, 179.0f);
    m_projectionDirty = true;
}

void Camera::setAspectRatio(float ratio) {
    if (ratio > 0.0f) {
        m_aspectRatio = ratio;
        m_projectionDirty = true;
    }
}

void Camera::setClipPlanes(float nearPlane, float farPlane) {
    if (nearPlane > 0.0f && farPlane > nearPlane) {
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        m_projectionDirty = true;
    }
}

void Camera::moveForward(float amount) {
    if (m_mode == CameraMode::Free) {
        m_position += m_front * amount * m_moveSpeed;
        m_viewDirty = true;
    }
}

void Camera::moveRight(float amount) {
    if (m_mode == CameraMode::Free) {
        m_position += m_right * amount * m_moveSpeed;
        m_viewDirty = true;
    }
}

void Camera::moveUp(float amount) {
    if (m_mode == CameraMode::Free) {
        m_position += m_worldUp * amount * m_moveSpeed;
        m_viewDirty = true;
    }
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    m_yaw += yawDelta * m_rotationSpeed;
    m_pitch += pitchDelta * m_rotationSpeed;

    // Constrain pitch
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    updateVectors();

    if (m_mode == CameraMode::Orbit) {
        // Update position based on orbit
        m_position = m_target - m_front * m_orbitDistance;
    }

    m_viewDirty = true;
}

void Camera::setOrbitDistance(float distance) {
    m_orbitDistance = std::max(0.1f, distance);
    if (m_mode == CameraMode::Orbit) {
        m_position = m_target - m_front * m_orbitDistance;
        m_viewDirty = true;
    }
}

void Camera::orbit(float yawDelta, float pitchDelta) {
    if (m_mode == CameraMode::Orbit) {
        rotate(yawDelta, pitchDelta);
    }
}

void Camera::zoom(float amount) {
    if (m_mode == CameraMode::Orbit) {
        m_orbitDistance = std::max(0.1f, m_orbitDistance - amount);
        m_position = m_target - m_front * m_orbitDistance;
        m_viewDirty = true;
    } else {
        // For free camera, adjust FOV
        setFOV(m_fov - amount);
    }
}

void Camera::reset() {
    m_position = glm::vec3(0.0f, 5.0f, 10.0f);
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_yaw = -90.0f;
    m_pitch = -20.0f;
    m_orbitDistance = 15.0f;
    m_fov = 45.0f;

    updateVectors();
    m_viewDirty = true;
    m_projectionDirty = true;
}

glm::vec3 Camera::screenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize, float depth) const {
    // Convert screen coordinates to normalized device coordinates
    glm::vec4 ndc;
    ndc.x = (2.0f * screenPos.x) / screenSize.x - 1.0f;
    ndc.y = 1.0f - (2.0f * screenPos.y) / screenSize.y;
    ndc.z = 2.0f * depth - 1.0f;
    ndc.w = 1.0f;

    // Inverse transform
    glm::mat4 invVP = glm::inverse(m_projectionMatrix * m_viewMatrix);
    glm::vec4 worldPos = invVP * ndc;

    if (std::abs(worldPos.w) > 0.0001f) {
        worldPos /= worldPos.w;
    }

    return glm::vec3(worldPos);
}

void Camera::updateVectors() {
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    m_front.x = std::cos(yawRad) * std::cos(pitchRad);
    m_front.y = std::sin(pitchRad);
    m_front.z = std::sin(yawRad) * std::cos(pitchRad);
    m_front = glm::normalize(m_front);

    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::updateViewMatrix() {
    if (m_mode == CameraMode::Orbit || m_mode == CameraMode::Follow) {
        m_viewMatrix = glm::lookAt(m_position, m_target, m_worldUp);
    } else {
        m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
    }
}

void Camera::updateProjectionMatrix() {
    m_projectionMatrix = glm::perspective(
        glm::radians(m_fov),
        m_aspectRatio,
        m_nearPlane,
        m_farPlane
    );
}

} // namespace pas::rendering
