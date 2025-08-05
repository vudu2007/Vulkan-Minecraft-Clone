#pragma once

#include "chunk.hpp"

#include "BS_thread_pool.hpp"
#include <GLM/gtx/hash.hpp>

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class World
{
  private:
    BS::thread_pool<> threadPool;
    std::mutex accessChunksMutex;

    FastNoiseLite terrainHeightNoise;
    unsigned seed;
    int chunkSize; // In blocks.

    // TODO: currently, a cache of chunks; will probably need an eviction policy to save memory;
    // maybe don't cache chunks at all and store world data in persistant memory and load them when needed;
    // maybe use a combination where inactive cached chunks are written to persistent memory.
    std::unordered_map<ChunkCenter, Chunk*> chunks;
    std::unordered_set<ChunkCenter> chunksToAdd;
    std::unordered_set<ChunkCenter> activeChunks;

    std::vector<std::function<void(const Chunk&)>> chunkLoadedCallbacks;
    std::vector<std::function<void(const Chunk&)>> chunkUnloadedCallbacks;
    std::vector<std::function<void()>> chunksChangedCallbacks;

    void runChunkLoadedCallbacks(const Chunk& chunk);
    void runChunkUnloadedCallbacks(const Chunk& chunk);

    bool isChunkActive(const ChunkCenter& cc) const;

  public:
    World(const unsigned seed, const int chunk_size, const unsigned num_threads = 1);
    ~World();

    void init(const glm::vec3& origin, const unsigned radius);

    std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr);

    void addChunk(const std::vector<glm::vec3> chunk_center);
    unsigned updateChunks(const glm::vec3& origin, const unsigned radius);

    void addBlock(const glm::vec3 block_pos);
    void removeBlock(const glm::vec3 block_pos);

    void addChunkLoadedCallback(const std::function<void(const Chunk&)>& callback);
    void clearChunkLoadedCallbacks();

    void addChunkUnloadedCallback(const std::function<void(const Chunk&)>& callback);
    void clearChunkUnloadedCallbacks();

    const ChunkCenter getPosToChunkCenter(const glm::vec3& pos) const;
    const Model getModel() const;
};
