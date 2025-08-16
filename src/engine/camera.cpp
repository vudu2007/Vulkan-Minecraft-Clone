#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

Camera::Camera(
    Window& window,
    const glm::vec3& eye_world,
    const glm::vec3& target_world,
    const glm::vec3& up_world,
    const float fov,
    const float aspect,
    const float z_near,
    const float z_far)
    : window(window), eye(eye_world), worldUp(up_world), fov(fov), aspect(aspect), zNear(z_near), zFar(z_far)
{
    window.addResizeCallback([this]() { this->updateAspectRatio(); });

    forward = glm::normalize(target_world - eye_world);
    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));

    // TODO: figure out initial Euler angles.
    const float pitch = 0.0f;
    const float yaw = 0.0f;
    const float roll = 0.0f;
    eulerAngles = glm::vec3(pitch, yaw, roll);
}

void Camera::updateAspectRatio()
{
    int width = 0, height = 0;
    window.getFrameBufferSize(width, height);
    aspect = static_cast<float>(width) / static_cast<float>(height);
}

void Camera::translate(const glm::vec3 translation)
{
    eye += translation;
}

void Camera::rotate(const float pitch, const float yaw, const float roll, const bool constrain_x_axis)
{
    eulerAngles += glm::radians(glm::vec3(pitch, yaw, roll));

    if (constrain_x_axis)
    {
        eulerAngles.x = glm::clamp(eulerAngles.x, minXAngleRad, maxXAngleRad);
    }

    // Adjust forward
    forward.x = glm::cos(eulerAngles.y) * glm::cos(eulerAngles.x);
    forward.y = glm::sin(eulerAngles.x);
    forward.z = glm::sin(eulerAngles.y) * glm::cos(eulerAngles.x);
    forward = glm::normalize(forward);

    // Update right and up
    right = glm::normalize(glm::cross(forward, worldUp));
    up = glm::normalize(glm::cross(right, forward));
}

void Camera::moveForward(const float units)
{
    translate(forward * units);
}

void Camera::moveBackward(const float units)
{
    translate(-forward * units);
}

void Camera::moveLeft(const float units)
{
    translate(-right * units);
}

void Camera::moveRight(const float units)
{
    translate(right * units);
}

void Camera::moveUp(const float units)
{
    translate(up * units);
}

void Camera::moveDown(const float units)
{
    translate(-up * units);
}

void Camera::moveForwardXZ(const float units)
{
    translate(glm::normalize(glm::vec3(forward.x, 0.0f, forward.z)) * units);
}

void Camera::moveBackwardXZ(const float units)
{
    translate(-glm::normalize(glm::vec3(forward.x, 0.0f, forward.z)) * units);
}

void Camera::moveLeftXZ(const float units)
{
    translate(-glm::normalize(glm::vec3(right.x, 0.0f, right.z)) * units);
}

void Camera::moveRightXZ(const float units)
{
    translate(glm::normalize(glm::vec3(right.x, 0.0f, right.z)) * units);
}

void Camera::moveUpXZ(const float units)
{
    translate(worldUp * units);
}

void Camera::moveDownXZ(const float units)
{
    translate(-worldUp * units);
}

glm::vec3 Camera::getEye() const
{
    return eye;
}

glm::vec3 Camera::getForward() const
{
    return forward;
}

glm::vec3 Camera::getUp() const
{
    return up;
}

glm::vec3 Camera::getRight() const
{
    return right;
}

glm::vec3 Camera::getWorldUp() const
{
    return worldUp;
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(eye, eye + forward, up);
}

glm::mat4 Camera::projMatrix() const
{
    return glm::perspective(fov, aspect, zNear, zFar);
}
