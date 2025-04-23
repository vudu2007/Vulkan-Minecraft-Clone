#pragma once

#include "engine/camera.hpp"
#include "engine/renderer/window.hpp"

class Player
{
  private:
    inline static const float DEFAULT_SPEED = 0.1f;

    Window& window;

    Camera camera;

    // Player attributes.
    float speed;

    // Mouse variables.
    float mouseSensitivity = 0.08f;
    float cursorPrevX = 0.0f;
    float cursorPrevY = 0.0f;

    void pollKeyboardControls();
    void eventKeyboardControls(const int key, const int scancode, const int action, const int mods);

  public:
    Player(Window& window, const float speed = DEFAULT_SPEED);

    void processInput();
    const Camera& getCamera() const;
};
