#ifndef VMC_SRC_ENGINE_CAMERA_HPP
#define VMC_SRC_ENGINE_CAMERA_HPP

#include "renderer/window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/glm.hpp>

class Camera
{
  protected:
    Window& window;

    glm::vec3 eye;
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;

    glm::vec3 worldUp;

    float fov = 0.0f;
    float aspect = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;

    glm::vec3 eulerAngles; // TODO: Use quaternions
    float minXAngleRad = glm::radians(-89.0f);
    float maxXAngleRad = glm::radians(89.0f);

    void updateAspectRatio();

  public:
    Camera(
        Window& window,
        const glm::vec3& eye_world,
        const glm::vec3& target_world,
        const glm::vec3& up_world,
        const float fov,
        const float aspect,
        const float z_near,
        const float z_far);

    void translate(const glm::vec3 translation);
    void rotate(const float pitch, const float yaw, const float roll, const bool constrain_x_axis);

    void moveForward(const float units);
    void moveBackward(const float units);
    void moveLeft(const float units);
    void moveRight(const float units);

    glm::vec3 getEye() const;

    glm::mat4 viewMatrix() const;
    glm::mat4 projMatrix() const;
};

class FpsCamera : public Camera
{
  private:
    float speed;

    // Mouse variables.
    float mouseSensitivity;
    float prevX = 0.0f;
    float prevY = 0.0f;

    void PolledKeyboardControls();
    void EventKeyboardControls(const int key, const int scancode, const int action, const int mods);

  public:
    FpsCamera(
        Window& window,
        const glm::vec3& eye_world,
        const glm::vec3& target_world,
        const glm::vec3& up_world,
        const float fov,
        const float aspect,
        const float z_near,
        const float z_far,
        const float speed,
        const float mouse_sensitivity);

    void processInput();
};

#endif // VMC_SRC_ENGINE_CAMERA_HPP
