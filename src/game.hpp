#pragma once

#include "engine/renderer/renderer.hpp"
#include "player.hpp"

#include <mutex>
#include <queue>
#include <stack>
#include <unordered_map>

class Game
{
  private:
    std::mutex updateMutex;

    static constexpr int CHUNK_SIZE = 16;
    static constexpr int MAX_NUM_BLOCKS_IN_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    static constexpr glm::vec3 DEFAULT_PLAYER_POS{0.0f, 2.0f, 0.0f};
    static constexpr int DEFAULT_PLAYER_RENDER_DISTANCE = 4;

    std::vector<unsigned> reusableIds; // TODO: std::stack doesn't like being down here.
    std::unordered_map<ChunkCenter, unsigned> chunkToVertexBufferId;

    Window window;
    Renderer renderer{window};
    World world{727, CHUNK_SIZE, static_cast<unsigned>(std::thread::hardware_concurrency() * 0.25)};
    Player player{window, world, DEFAULT_PLAYER_POS, 4.0f, DEFAULT_PLAYER_RENDER_DISTANCE};

    std::queue<Chunk*> chunksToLoad;
    std::queue<Chunk*> chunksToUnload;

    void loadChunkModel(const Chunk& chunk);
    void unloadChunkModel(const Chunk& chunk);

  public:
    void run();
};
