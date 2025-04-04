#ifndef VMC_SRC_GAME_HPP
#define VMC_SRC_GAME_HPP

#include "engine/renderer/renderer.hpp"

class Game
{
  private:
    Window window;
    Renderer renderer{window};

  public:
    void run();
};

#endif // VMC_SRC_GAME_HPP
