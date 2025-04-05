#include "camera.hpp"

#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtx/string_cast.hpp>

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

glm::vec3 Camera::getEye() const
{
    return eye;
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(eye, eye + forward, up);
}

glm::mat4 Camera::projMatrix() const
{
    return glm::perspective(fov, aspect, zNear, zFar);
}

void FpsCamera::PolledKeyboardControls()
{
    if (window.getKeyboardKey(GLFW_KEY_W) == GLFW_PRESS)
    {
        const glm::vec3 direction(forward.x, 0.0f, forward.z);
        translate(glm::normalize(direction) * speed);
    }
    if (window.getKeyboardKey(GLFW_KEY_S) == GLFW_PRESS)
    {
        const glm::vec3 direction(forward.x, 0.0f, forward.z);
        translate(-glm::normalize(direction) *speed);
    }
    if (window.getKeyboardKey(GLFW_KEY_A) == GLFW_PRESS)
    {
        const glm::vec3 direction(right.x, 0.0f, right.z);
        translate(-glm::normalize(direction) *speed);
    }
    if (window.getKeyboardKey(GLFW_KEY_D) == GLFW_PRESS)
    {
        const glm::vec3 direction(right.x, 0.0f, right.z);
        translate(glm::normalize(direction) * speed);
    }
    if (window.getKeyboardKey(GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        translate(worldUp * speed);
    }
    if (window.getKeyboardKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        translate(-worldUp * speed);
    }
}

void FpsCamera::EventKeyboardControls(const int key, const int scancode, const int action, const int mods)
{
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
    {
        if (window.getInputMode(GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
        {
            window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            window.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

FpsCamera::FpsCamera(
    Window& window,
    const glm::vec3& eye_world,
    const glm::vec3& target_world,
    const glm::vec3& up_world,
    const float fov,
    const float aspect,
    const float z_near,
    const float z_far,
    const float speed,
    const float mouse_sensitivity)
    : Camera(window, eye_world, target_world, up_world, fov, aspect, z_near, z_far), speed(speed),
      mouseSensitivity(mouse_sensitivity)
{
    window.addKeyCallback([this](int key, int scancode, int action, int mods) {
        this->EventKeyboardControls(key, scancode, action, mods);
    });
}

void FpsCamera::processInput()
{
    // Keyboard input.
    PolledKeyboardControls();

    // Mouse input.
    double x = 0.0, y = 0.0;
    window.getCursorPosition(x, y);
    if (window.getInputMode(GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        const float delta_x = (static_cast<float>(x) - prevX) * mouseSensitivity;
        const float delta_y = -(static_cast<float>(y) - prevY) * mouseSensitivity;

        if (delta_x != 0.0f || delta_y != 0.0f)
        {
            rotate(delta_y, delta_x, 0.0f, true);
        }
    }
    prevX = static_cast<float>(x);
    prevY = static_cast<float>(y);
}
