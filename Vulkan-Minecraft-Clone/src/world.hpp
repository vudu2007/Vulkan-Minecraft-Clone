#pragma once

#include "chunk.hpp"
#include "player.hpp"

#include <GLM/gtx/hash.hpp>

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class World
{
  private:
    using ChunkCoord = glm::vec2; // Center position of chunk floored as a string.

    std::mutex addChunkMutex;
    std::unordered_set<ChunkCoord> chunksToAdd;

    SimplexNoise noise;
    unsigned seed;
    int chunkSize; // In blocks.

    // TODO: currently, a cache of chunks; will probably need an eviction policy to save memory;
    // maybe don't cache chunks at all and store world data in persistant memory and load them when needed;
    // maybe use a combination where inactive cached chunks are written to persistant memory.
    std::unordered_map<ChunkCoord, Chunk*> chunks;
    std::unordered_map<ChunkCoord, Chunk*> activeChunks;

    const glm::vec2 posToChunkCenter(const glm::vec3 pos) const;

  public:
    World(const unsigned seed, const int chunk_size, const Player& player);
    ~World();

    void update(const Player& player);
    void addChunk(const std::vector<glm::vec2> chunk_center);
    unsigned updateChunks(const Player& player);

    const std::vector<const Chunk*> getActiveChunks() const;
};
