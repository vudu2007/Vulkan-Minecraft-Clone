#pragma once

#include "block.hpp"
#include "engine/camera.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/renderer/window.hpp"
#include "world.hpp"

class Player
{
  private:
    inline static const float DEFAULT_SPRINT_MULTIPLIER = 3.0f;

    Window& window;

    World& world;

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

  public:
    const Block* activeBlock = nullptr;

    Player(Window& window, World& world, const glm::vec3& pos, const float speed, const unsigned render_distance);

    void processInput();

    const Camera& getCamera() const;
    const glm::vec3 getPosition() const;
    const unsigned getRenderDistance() const;
    const Ray& getRay() const;
};
