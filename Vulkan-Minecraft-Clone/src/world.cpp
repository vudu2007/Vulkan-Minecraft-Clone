#include "world.hpp"

#include <thread>
#include <unordered_set>

const glm::vec2 World::posToChunkCenter(const glm::vec3 pos) const
{
    const float fp_chunk_size = static_cast<float>(chunkSize);
    const float stride = fp_chunk_size;
    const float x = std::floor((pos.x / fp_chunk_size + 0.5f)) * stride;
    const float z = std::floor((pos.z / fp_chunk_size + 0.5f)) * stride;
    return glm::vec2(x, z);
}

World::World(const unsigned seed, const int chunk_size) : noise(seed), seed(seed), chunkSize(chunk_size)
{
}

World::~World()
{
    for (auto& chunk : chunks)
    {
        free(chunk.second);
    }
}

void World::update(const Player& player)
{
}

void World::addChunk(const ChunkCoord cc, const glm::vec2 chunk_center)
{
    Chunk* chunk = new Chunk{noise, chunk_center, chunkSize};

    {
        std::lock_guard<std::mutex> lock(addChunkMutex);
        chunks.emplace(cc, chunk);
        activeChunks.emplace(cc, chunks[cc]);
        chunksToAdd.erase(cc);
    }
}

void World::updateChunks(const Player& player)
{
    // TODO: change to update in 3D.
    // Load all chunks visible to the player.
    const int render_distance = player.getRenderDistance();
    const float offset = static_cast<float>(render_distance * chunkSize);

    std::unordered_set<ChunkCoord> inactive_chunks;
    for (const auto& entry : activeChunks)
    {
        inactive_chunks.emplace(entry.first);
    }

    bool active_chunks_changed = false;
    float x = player.getPosition().x - offset;
    for (int i = 0; i <= (render_distance * 2); ++i)
    {
        float z = player.getPosition().z - offset;
        for (int j = 0; j <= (render_distance * 2); ++j)
        {
            const glm::vec2 chunk_center = posToChunkCenter({x, 0.0f, z});
            const ChunkCoord cc = chunk_center / static_cast<float>(chunkSize);
            inactive_chunks.erase(cc);
            if (!activeChunks.contains(cc))
            {
                active_chunks_changed = true;
                if (!chunks.contains(cc))
                {
                    // Create the chunk asynchrounously and add it later.
                    if (!chunksToAdd.contains(cc))
                    {
                        chunksToAdd.emplace(cc);
                        // TODO: use thread pool to avoid overhead of creating a thread again and again.
                        std::thread(&World::addChunk, this, cc, chunk_center).detach();
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

    {
        std::lock_guard<std::mutex> lock(addChunkMutex);

        // Remove now inactive chunks.
        for (const auto& cc : inactive_chunks)
        {
            activeChunks.erase(cc);
        }

        // TODO: merge on a separate thread or something since it's expensive. Or find a new way to merge.
        //// Merge active chunks.
        // if (active_chunks_changed)
        //{
        //     for (auto& entry : activeChunks)
        //     {
        //         const ChunkCoord& cc = entry.first;
        //         Chunk& chunk = *(entry.second);
        //
        //         // Merge with neighboring chunks.
        //         std::vector<glm::vec2> neighbor_ccs{
        //             (cc + glm::vec2(-1.0f, 0.0f)), // Left.
        //             (cc + glm::vec2(1.0f, 0.0f)),  // Right.
        //             (cc + glm::vec2(0.0f, -1.0f)), // Front.
        //             (cc + glm::vec2(0.0f, 1.0f)),  // Back.
        //         };
        //         std::vector<Chunk*> neighbor_chunks;
        //         neighbor_chunks.reserve(neighbor_ccs.size());
        //
        //         for (const auto& neighbor_cc : neighbor_ccs)
        //         {
        //             if (activeChunks.contains(neighbor_cc))
        //             {
        //                 neighbor_chunks.push_back(activeChunks[neighbor_cc]);
        //             }
        //         }
        //
        //         chunk.merge(neighbor_chunks);
        //     }
        // }
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
