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
    const float x = std::floor((pos.x / fp_chunk_size + 0.5f)) * stride;
    const float z = std::floor((pos.z / fp_chunk_size + 0.5f)) * stride;
    return World::ChunkCenter(x, z);
}

World::World(const unsigned seed, const int chunk_size, const Player& player)
    : noise(seed), seed(seed), chunkSize(chunk_size)
{
    std::cout << ">>> Loading world with seed (" << seed << ")..." << std::endl;

    const unsigned total_num_chunks = updateChunks(player);

    // TODO: main thread provides updates to user here.
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

void World::update(Player& player)
{
    // Update what is in the player's reach.
    // TODO: make sure to check all chunks that are within the palyer's reach.
    const glm::vec2 chunk_center = posToChunkCenter(player.getPosition());
    const ChunkCoord cc = chunkCenterToChunkCoord(chunk_center);
    assert(activeChunks.contains(cc));
    const Chunk* chunk = activeChunks[cc];
    player.activeBlock = chunk->getBlockInReach(player);
    if (player.activeBlock != nullptr)
    {
        std::cout << "\r" << "block in reach at position: (" << player.activeBlock->position.x << ", "
                  << player.activeBlock->position.y << ", " << player.activeBlock->position.z << ")";
    }
    else
    {
        std::cout << "\x1b[2K\rno block in reach";
    }
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

unsigned World::updateChunks(const Player& player)
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

    std::vector<glm::vec2> chunk_centers;
    bool add_chunks = false;

    float x = player.getPosition().x - offset;
    for (int i = 0; i <= (render_distance * 2); ++i)
    {
        float z = player.getPosition().z - offset;
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

    return chunk_centers.size();
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
