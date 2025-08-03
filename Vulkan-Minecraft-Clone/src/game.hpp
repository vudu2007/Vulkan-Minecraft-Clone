#pragma once

#include "engine/renderer/renderer.hpp"
#include "player.hpp"

#include <mutex>
#include <queue>
#include <stack>

class Game
{
  private:
    std::mutex updateMutex;

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int MAX_NUM_BLOCKS_IN_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    static constexpr glm::vec3 DEFAULT_PLAYER_POS{0.0f};
    static constexpr int DEFAULT_PLAYER_RENDER_DISTANCE = 4;

    Window window;
    Renderer renderer{window};
    World world{727, CHUNK_SIZE};
    Player player{window, world, DEFAULT_PLAYER_POS, 0.1f, DEFAULT_PLAYER_RENDER_DISTANCE};

    std::stack<unsigned> reusableIds;
    std::unordered_map<ChunkCenter, unsigned> chunkToVertexBufferId;

    std::queue<Chunk*> chunksToLoad;
    std::queue<Chunk*> chunksToUnload;

    void loadChunkModel(const Chunk& chunk);
    void unloadChunkModel(const Chunk& chunk);

  public:
    void run();
};
