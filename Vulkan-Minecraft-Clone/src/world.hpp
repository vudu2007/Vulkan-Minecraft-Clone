#pragma once

#include "chunk.hpp"
#include "player.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class World
{
  private:
    using ChunkCoord = std::string; // Center position of chunk floored as a string.

    SimplexNoise noise;
    unsigned seed;
    int chunkSize; // In blocks.

    std::unordered_map<ChunkCoord, Chunk> chunks;
    std::unordered_map<ChunkCoord, Chunk> activeChunks;

    const glm::vec2 posToChunkCenter(const glm::vec3 pos) const;

  public:
    World(const unsigned seed, const int chunk_size);

    void update(const Player& player);
    void updateChunks(const Player& player);

    const std::vector<Chunk> getActiveChunks() const;
};
