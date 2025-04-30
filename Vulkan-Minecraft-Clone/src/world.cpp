#include "world.hpp"

#include <iostream>

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

void World::update(const Player& player)
{
    activeChunks.clear();

    // Load all chunks visible to the player.
    const glm::vec2 chunk_center = posToChunkCenter(player.getPosition());
    const ChunkCoord cc = glm::to_string(chunk_center);
    if (!activeChunks.contains(cc))
    {
        if (chunks.contains(cc))
        {
            activeChunks.emplace(cc, chunks[cc]);
        }
        else
        {
            activeChunks.emplace(cc, Chunk{noise, chunk_center, chunkSize});
        }
    }
}

const std::vector<Chunk> World::getActiveChunks() const
{
    std::vector<Chunk> active_chunks;
    for (const auto& chunk_info : activeChunks)
    {
        active_chunks.push_back(chunk_info.second);
    }
    return active_chunks;
}
