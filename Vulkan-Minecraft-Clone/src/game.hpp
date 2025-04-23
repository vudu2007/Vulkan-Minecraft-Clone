#ifndef VMC_SRC_GAME_HPP
#define VMC_SRC_GAME_HPP

#include "engine/renderer/renderer.hpp"
#include "player.hpp"

class Game
{
  private:
    Window window;
    Renderer renderer{window};
    Player player{window};

  public:
    void run();
};

#endif // VMC_SRC_GAME_HPP
