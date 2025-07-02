#pragma once

#include "chunk.hpp"

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
    using ChunkCenter = glm::vec2;

    std::mutex activeChunksMutex;
    std::unordered_set<ChunkCoord> chunksToAdd;

    FastNoiseLite terrainHeightNoise;
    unsigned seed;
    int chunkSize; // In blocks.

    // TODO: currently, a cache of chunks; will probably need an eviction policy to save memory;
    // maybe don't cache chunks at all and store world data in persistant memory and load them when needed;
    // maybe use a combination where inactive cached chunks are written to persistent memory.
    std::unordered_map<ChunkCoord, Chunk*> chunks;
    std::unordered_map<ChunkCoord, Chunk*> activeChunks;

    const ChunkCoord chunkCenterToChunkCoord(const ChunkCenter& chunk_center) const;
    const ChunkCenter posToChunkCenter(const glm::vec3& pos) const;

  public:
    World(const unsigned seed, const int chunk_size, const glm::vec3& origin, const unsigned radius);
    ~World();

    std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr);

    void addChunk(const std::vector<glm::vec2> chunk_center);
    unsigned updateChunks(const glm::vec3& origin, const unsigned radius);

    void addBlock(const glm::vec3 block_pos);
    void removeBlock(const glm::vec3 block_pos);

    const std::vector<Chunk*> getActiveChunks() const;
    const Model getModel() const;
};
