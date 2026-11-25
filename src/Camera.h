#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement
{
    Forward,
    Backward,
    Left,
    Right
};

class Camera
{
public:
    static constexpr float kDefaultYaw = -90.0f;
    static constexpr float kDefaultPitch = 0.0f;
    static constexpr float kDefaultSpeed = 5.0f;
    static constexpr float kDefaultSensitivity = 0.35f;

    Camera(glm::vec3 position, glm::vec3 worldUp, float yaw = kDefaultYaw, float pitch = kDefaultPitch);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

    void ProcessKeyboardInput(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

private:
    void UpdateCameraVectors();

    glm::vec3 m_position;
    glm::vec3 m_direction{ 0.0f, 0.0f, -1.0f };
    glm::vec3 m_up{ 0.0f, 1.0f, 0.0f };
    glm::vec3 m_right{ 1.0f, 0.0f, 0.0f };
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;
    float m_speed = kDefaultSpeed;
    float m_mouseSensitivity = kDefaultSensitivity;
};
