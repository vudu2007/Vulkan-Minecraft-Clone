#ifndef VMC_SRC_GAME_HPP
#define VMC_SRC_GAME_HPP

#include "engine/renderer/renderer.hpp"
#include "player.hpp"

class Game
{
  private:
    static const int CHUNK_SIZE = 16;
    inline static const glm::vec3 DEFAULT_PLAYER_POS{0.0f};
    static const int DEFAULT_PLAYER_RENDER_DISTANCE = 2;

    Window window;
    Renderer renderer{window};
    World world{727, CHUNK_SIZE, DEFAULT_PLAYER_POS, DEFAULT_PLAYER_RENDER_DISTANCE};
    Player player{window, world, DEFAULT_PLAYER_POS, 0.1f, DEFAULT_PLAYER_RENDER_DISTANCE};

    unsigned terrainVertBufferIdx;
    void updateTerrain();

  public:
    void run();
};

#endif // VMC_SRC_GAME_HPP
