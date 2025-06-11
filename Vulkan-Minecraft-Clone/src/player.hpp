#pragma once

#include "block.hpp"
#include "engine/camera.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/renderer/window.hpp"

class Player
{
  private:
    inline static const float DEFAULT_SPEED = 0.1f;
    inline static const glm::vec3 DEFAULT_POSITION{0.0f};
    inline static unsigned DEFAULT_RENDER_DISTANCE = 1;
    inline static const float DEFAULT_SPRINT_MULTIPLIER = 3.0f;

    Window& window;

    Camera camera;
    Ray reach;

    // Player attributes.
    glm::vec3 position;
    float speed;
    unsigned renderDistance;

    // Mouse variables.
    float mouseSensitivity = 0.08f;
    float cursorPrevX = 0.0f;
    float cursorPrevY = 0.0f;

    void pollKeyboardControls();
    void eventKeyboardControls(const int key, const int scancode, const int action, const int mods);

    void eventMouseControls(const int button, const int action, const int mods);

    // Event handling.
    std::vector<std::function<void(Player&)>> moveCallbacks;

  public:
    const Block* activeBlock = nullptr;

    Player(
        Window& window,
        const glm::vec3& pos = DEFAULT_POSITION,
        const float speed = DEFAULT_SPEED,
        const unsigned render_distance = DEFAULT_RENDER_DISTANCE);

    void processInput();

    const Camera& getCamera() const;
    const glm::vec3 getPosition() const;
    const unsigned getRenderDistance() const;
    const Ray& getRay() const;

    void addMoveCallback(const std::function<void(Player&)>& callback);
    void clearMoveCallbacks();
};
