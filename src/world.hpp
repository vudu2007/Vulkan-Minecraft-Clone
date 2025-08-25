#pragma once

#include "chunk.hpp"

#include "engine/frustum.hpp"

#include "BS_thread_pool.hpp"
#include <glm/gtx/hash.hpp>

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class World
{
  private:
    BS::thread_pool<> threadPool;
    std::shared_mutex chunksMutex;

    FastNoiseLite terrainHeightNoise;
    unsigned seed;
    int chunkSize; // In blocks.

    float gravity = -9.8f;

    // TODO: currently, a cache of chunks; will probably need an eviction policy to save memory;
    // maybe don't cache chunks at all and store world data in persistant memory and load them when needed;
    // maybe use a combination where inactive cached chunks are written to persistent memory.
    std::unordered_map<ChunkCenter, Chunk*> chunks;
    std::unordered_set<ChunkCenter> chunksToAdd;
    std::unordered_set<ChunkCenter> activeChunks;
    std::unordered_set<ChunkCenter> chunksToShow;
    std::unordered_set<ChunkCenter> visibleChunks;

    std::vector<std::function<void(const Chunk&)>> chunkLoadedCallbacks;
    std::vector<std::function<void(const Chunk&)>> chunkUnloadedCallbacks;
    std::vector<std::function<void()>> chunksChangedCallbacks;

    void runChunkLoadedCallbacks(const Chunk& chunk);
    void runChunkUnloadedCallbacks(const Chunk& chunk);

    bool isChunkActive(const ChunkCenter& cc) const;

    std::array<Chunk*, 6> getNeighboringChunks(const ChunkCenter& cc) const;

    void editBlock(const glm::vec3 block_pos, const bool should_add);

  public:
    World(const unsigned seed, const int chunk_size, const unsigned num_threads = 1);
    ~World();

    void init(const glm::vec3& origin, const unsigned radius);

    std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr);
    bool doesEntityIntersect(
        const glm::vec3& pos,
        const glm::vec3& velocity,
        const float delta,
        const Aabb3d& hitbox,
        float& new_delta,
        glm::vec3* normal = nullptr);

    void addChunk(const std::vector<glm::vec3> chunk_center);

    void draw(const glm::vec3& origin, const unsigned radius, const Frustum& frustum);
    unsigned updateChunks(const glm::vec3& origin, const unsigned radius);

    void addBlock(const glm::vec3 block_pos);
    void removeBlock(const glm::vec3 block_pos);

    void addChunkLoadedCallback(const std::function<void(const Chunk&)>& callback);
    void clearChunkLoadedCallbacks();

    void addChunkUnloadedCallback(const std::function<void(const Chunk&)>& callback);
    void clearChunkUnloadedCallbacks();

    const ChunkCenter getPosToChunkCenter(const glm::vec3& pos) const;

    float getGravity() const;
};
