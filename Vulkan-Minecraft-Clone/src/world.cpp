#include "world.hpp"

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

void World::update(const Player& player)
{
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

    float x = player.getPosition().x - offset;
    for (int i = 0; i <= (render_distance * 2); ++i)
    {
        float z = player.getPosition().z - offset;
        for (int j = 0; j <= (render_distance * 2); ++j)
        {
            const glm::vec2 chunk_center = posToChunkCenter({x, 0.0f, z});
            const ChunkCoord cc = glm::to_string(chunk_center);
            inactive_chunks.erase(cc);
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
            z += chunkSize;
        }
        x += chunkSize;
    }

    // Remove now inactive chunks.
    for (const auto& cc : inactive_chunks)
    {
        activeChunks.erase(cc);
    }
}

const std::vector<Chunk> World::getActiveChunks() const
{
    std::vector<Chunk> active_chunks;
    for (const auto& entry : activeChunks)
    {
        active_chunks.push_back(entry.second);
    }
    return active_chunks;
}
