#ifndef VMC_SRC_GAME_HPP
#define VMC_SRC_GAME_HPP

#include "engine/camera.hpp"
#include "engine/renderer/renderer.hpp"

class Game
{
  private:
    Window window;
    FpsCamera camera{
        window,
        glm::vec3(2, 2, 2),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0),
        glm::radians(45.0f),
        (static_cast<float>(Window::DEFAULT_WIDTH) / static_cast<float>(Window::DEFAULT_HEIGHT)),
        0.1f,
        100.0f,
        0.1f,
        0.08f};
    Renderer renderer{window, camera};

  public:
    void run();
};

#endif // VMC_SRC_GAME_HPP
