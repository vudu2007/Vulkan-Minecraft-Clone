#include "world.hpp"

#include <cassert>
#include <iostream>
#include <thread>
#include <unordered_set>

const World::ChunkCoord World::chunkCenterToChunkCoord(const ChunkCenter& chunk_center) const
{
    return chunk_center / static_cast<float>(chunkSize);
}

const World::ChunkCenter World::posToChunkCenter(const glm::vec3& pos) const
{
    const float fp_chunk_size = static_cast<float>(chunkSize);
    const float stride = fp_chunk_size;
    const float x = std::floorf(((pos.x + 0.5f) / fp_chunk_size + 0.5f)) * stride;
    const float z = std::floorf(((pos.z + 0.5f) / fp_chunk_size + 0.5f)) * stride;
    return World::ChunkCenter(x, z);
}

World::World(const unsigned seed, const int chunk_size, const glm::vec3& origin, const unsigned radius)
    : noise(seed), seed(seed), chunkSize(chunk_size)
{
    std::cout << ">>> Loading world with seed (" << seed << ")..." << std::endl;

    const unsigned total_num_chunks = updateChunks(origin, radius);

    // Report an update every second.
    do
    {
        std::cout << "  Loaded " << (total_num_chunks - chunksToAdd.size()) << "/" << total_num_chunks << " chunks."
                  << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!chunksToAdd.empty());

    std::cout << "  Loaded " << (total_num_chunks - chunksToAdd.size()) << "/" << total_num_chunks << " chunks."
              << std::endl;

    std::cout << ">>> Finished loading world!" << std::endl;
}

World::~World()
{
    for (auto& chunk : chunks)
    {
        free(chunk.second);
    }
}

std::optional<glm::vec3> World::getReachableBlock(const Ray& ray)
{
    std::optional<glm::vec3> reachable_block_pos;

    // Under the assumption that the player's reach is never infinity.

    // Check the chunk that contains the ray's origin (the player's position).
    const ChunkCenter chunk_center = posToChunkCenter(ray.getOrigin());
    ChunkCoord cc = chunkCenterToChunkCoord(chunk_center);
    reachable_block_pos = activeChunks[cc]->getReachableBlock(ray);
    if (reachable_block_pos.has_value())
    {
        return reachable_block_pos;
    }

    // Check the chunk that contains the ray's direction at its max distance (where the player is looking).
    const ChunkCenter next_chunk_center = posToChunkCenter(ray.getOrigin() + ray.getDirection() * ray.getMax());
    if (next_chunk_center == chunk_center)
    {
        return reachable_block_pos;
    }
    cc = chunkCenterToChunkCoord(next_chunk_center);
    reachable_block_pos = activeChunks[cc]->getReachableBlock(ray);

    return reachable_block_pos;
}

void World::addChunk(const std::vector<glm::vec2> chunk_centers)
{
    for (const auto& chunk_center : chunk_centers)
    {
        Chunk* chunk = new Chunk{noise, chunk_center, chunkSize};
        const ChunkCoord cc = chunkCenterToChunkCoord(chunk_center);

        {
            std::lock_guard<std::mutex> lock(addChunkMutex);
            chunks.emplace(cc, chunk);
            activeChunks.emplace(cc, chunks[cc]);
            chunksToAdd.erase(cc);
        }
    }
}

unsigned World::updateChunks(const glm::vec3& origin, const unsigned radius)
{
    // TODO: change to update in 3D.
    // Load all chunks visible to the player.
    const int render_distance = static_cast<int>(radius);
    const float offset = static_cast<float>(render_distance * chunkSize);

    std::unordered_set<ChunkCoord> inactive_chunks;
    for (const auto& entry : activeChunks)
    {
        inactive_chunks.emplace(entry.first);
    }

    std::vector<glm::vec2> chunk_centers;
    bool add_chunks = false;

    float x = origin.x - offset;
    for (int i = 0; i <= (render_distance * 2); ++i)
    {
        float z = origin.z - offset;
        for (int j = 0; j <= (render_distance * 2); ++j)
        {
            const ChunkCenter chunk_center = posToChunkCenter({x, 0.0f, z});
            const ChunkCoord cc = chunkCenterToChunkCoord(chunk_center);
            inactive_chunks.erase(cc);
            if (!activeChunks.contains(cc))
            {
                if (!chunks.contains(cc))
                {
                    // Create the chunk asynchrounously and add it later.
                    if (!chunksToAdd.contains(cc))
                    {
                        chunksToAdd.emplace(cc);
                        chunk_centers.emplace_back(chunk_center);
                        add_chunks = true;
                    }
                }
                else
                {
                    activeChunks.emplace(cc, chunks[cc]);
                }
            }
            z += chunkSize;
        }
        x += chunkSize;
    }

    if (add_chunks)
    {
        // TODO: use thread pool to avoid overhead of creating a thread again and again.
        std::thread(&World::addChunk, this, chunk_centers).detach();
    }

    {
        std::lock_guard<std::mutex> lock(addChunkMutex);

        // Remove now inactive chunks.
        for (const auto& cc : inactive_chunks)
        {
            activeChunks.erase(cc);
        }
    }

    return static_cast<unsigned>(chunk_centers.size());
}

void World::removeBlock(const glm::vec3 block_pos)
{
    const ChunkCenter chunk_center = posToChunkCenter(block_pos); // ID is currently the block's position.
    ChunkCoord cc = chunkCenterToChunkCoord(chunk_center);
    activeChunks[cc]->removeBlock(block_pos);

    // Try it on neighboring chunks.
    std::vector<glm::vec3> neighbors = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
    };
    for (const auto& offset : neighbors)
    {
        const ChunkCenter chunk_center = posToChunkCenter(block_pos + offset);
        ChunkCoord cc = chunkCenterToChunkCoord(chunk_center);
        activeChunks[cc]->removeBlock(block_pos);
    }
}

const std::vector<const Chunk*> World::getActiveChunks() const
{
    std::vector<const Chunk*> active_chunks;
    for (const auto& entry : activeChunks)
    {
        active_chunks.push_back(entry.second);
    }
    return active_chunks;
}
