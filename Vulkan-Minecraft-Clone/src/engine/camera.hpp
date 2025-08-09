#ifndef VMC_SRC_ENGINE_CAMERA_HPP
#define VMC_SRC_ENGINE_CAMERA_HPP

#include "renderer/window.hpp"

#include "usage/glm-usage.hpp"

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

    // Translation: free movement.
    void moveForward(const float units);
    void moveBackward(const float units);
    void moveLeft(const float units);
    void moveRight(const float units);
    void moveUp(const float units);
    void moveDown(const float units);

    // Translation: move along the XZ-plane.
    void moveForwardXZ(const float units);
    void moveBackwardXZ(const float units);
    void moveLeftXZ(const float units);
    void moveRightXZ(const float units);
    void moveUpXZ(const float units);
    void moveDownXZ(const float units);

    glm::vec3 getEye() const;
    glm::vec3 getForward() const;
    glm::vec3 getUp() const;
    glm::vec3 getRight() const;
    glm::vec3 getWorldUp() const;

    glm::mat4 viewMatrix() const;
    glm::mat4 projMatrix() const;
};

#endif // VMC_SRC_ENGINE_CAMERA_HPP
