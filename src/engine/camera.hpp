#pragma once

#include "frustum.hpp"
#include "usage/glm-usage.hpp"
#include "window.hpp"

class Camera
{
  public:
    Camera(
        Window& window,
        const glm::vec3& eye_world,
        const glm::vec3& target_world,
        const glm::vec3& up_world,
        float fov_y,
        float aspect,
        float z_near,
        float z_far);

    void translate(const glm::vec3& units);
    void rotate(float pitch, float yaw, float roll, bool constrain_x_axis);

    // Translation: free movement.
    void moveForward(float units);
    void moveBackward(float units);
    void moveLeft(float units);
    void moveRight(float units);
    void moveUp(float units);
    void moveDown(float units);

    // Translation: move along the XZ-plane.
    void moveForwardXZ(float units);
    void moveBackwardXZ(float units);
    void moveLeftXZ(float units);
    void moveRightXZ(float units);
    void moveUpXZ(float units);
    void moveDownXZ(float units);

    glm::vec3 getEye() const;
    glm::vec3 getForward() const;
    glm::vec3 getUp() const;
    glm::vec3 getRight() const;
    glm::vec3 getWorldUp() const;
    float getFovYRad() const;
    void setFovYRad(float radians);
    float getFovYDeg() const;
    void setFovYDeg(float degrees);
    float getAspect() const;
    float getNear() const;
    float getFar() const;
    const Frustum& getFrustum() const;

    glm::mat4 viewMatrix() const;
    glm::mat4 projMatrix() const;

  private:
    Window& window;

    glm::vec3 worldUp;

    glm::vec3 eye;
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    float fovY = 0.0f;
    float aspect = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;

    glm::vec3 eulerAngles; // TODO: Use quaternions?
    float minXAngleRad = glm::radians(-89.0f);
    float maxXAngleRad = glm::radians(89.0f);

    Frustum frustum;

    void updateAspectRatio();
};
