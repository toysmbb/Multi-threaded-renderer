#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
    : m_position(position), m_worldUp(worldUp), m_yaw(yaw), m_pitch(pitch)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_direction, m_up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const
{
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

void Camera::UpdateCameraVectors()
{
    const float yaw = glm::radians(m_yaw);
    const float pitch = glm::radians(m_pitch);

    m_direction = glm::normalize(glm::vec3(std::cos(yaw) * std::cos(pitch), std::sin(pitch),
                                           std::sin(yaw) * std::cos(pitch)));
    m_right = glm::normalize(glm::cross(m_direction, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_direction));
}

void Camera::ProcessKeyboardInput(CameraMovement direction, float deltaTime)
{
    const float velocity = m_speed * deltaTime;

    switch (direction)
    {
    case CameraMovement::Forward:
        m_position += m_direction * velocity;
        break;
    case CameraMovement::Backward:
        m_position -= m_direction * velocity;
        break;
    case CameraMovement::Left:
        m_position -= m_right * velocity;
        break;
    case CameraMovement::Right:
        m_position += m_right * velocity;
        break;
    }
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
    m_yaw += xOffset * m_mouseSensitivity;
    m_pitch += yOffset * m_mouseSensitivity;

    if (constrainPitch)
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    UpdateCameraVectors();
}
