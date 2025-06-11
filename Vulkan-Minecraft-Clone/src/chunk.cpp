#include "chunk.hpp"

#include "engine/physics/collision-handler.hpp"

#include <algorithm>
#include <unordered_set>

Chunk::Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size) : position(center_pos), size(size)
{
    const int x_start = static_cast<int>(center_pos.x - (size / static_cast<float>(2)));
    const int z_start = static_cast<int>(center_pos.y - (size / static_cast<float>(2)));

    int min_height = -100; // std::numeric_limits<int>::max(); // TODO

    // Keep track of blocks in neighboring chunks to discard later because they shouldn't exist in this chunk.
    // Will use these blocks to figure out whether blocks on edge of this chunk should be visible.
    std::unordered_set<BlockId> block_ids_to_discard;

    // Start at the corner of the chunk and offset it by -1 to figure out the blocks in neighboring chunks.
    // End at the corner with an offset of +1 or a condition `<=` to reach blocks in neighboring chunks.
    blockMap = std::make_unique<std::unordered_map<glm::vec3, Block>>();
    for (int z = z_start - 1; z <= z_start + size; ++z)
    {
        for (int x = x_start - 1; x <= x_start + size; ++x)
        {
            int max_height = static_cast<int>(
                std::floorf(noise.getFractal2D(static_cast<float>(x), static_cast<float>(z), 5, 20.0f, 0.0035f)));

            min_height = std::min(min_height, max_height);

            for (int y = min_height; y <= max_height; ++y)
            {
                const glm::vec3 position(static_cast<float>(x), y, static_cast<float>(z));
                const BlockId block_id = position;
                blockMap->emplace(block_id, position);

                const bool is_z_edge = (z == z_start - 1) || (z == z_start + size);
                const bool is_x_edge = (x == x_start - 1) || (x == x_start + size);
                if (is_z_edge || is_x_edge)
                {
                    block_ids_to_discard.emplace(block_id);
                }
            }
        }
    }

    for (auto& map : *blockMap)
    {
        const BlockId& block_id = map.first;
        if (block_ids_to_discard.contains(block_id))
        {
            continue;
        }

        Block& block = map.second;
        const glm::vec3 top = block.position + glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 bottom = block.position + glm::vec3(0.0f, -1.0f, 0.0f);
        const glm::vec3 left = block.position + glm::vec3(-1.0f, 0.0f, 0.0f);
        const glm::vec3 right = block.position + glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::vec3 front = block.position + glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 back = block.position + glm::vec3(0.0f, 0.0f, 1.0f);

        const bool has_top = blockMap->contains(top);
        const bool has_bottom = blockMap->contains(bottom);
        const bool has_left = blockMap->contains(left);
        const bool has_right = blockMap->contains(right);
        const bool has_front = blockMap->contains(front);
        const bool has_back = blockMap->contains(back);

        if (has_top && has_bottom && has_left && has_right && has_front && has_back)
        {
            //  TODO: might not be necessary to store hidden blocks for now;
            //  a potential idea is to generate the blocks after the player
            //  modifies the world (place/break blocks) so that a chunk
            //  takes less memory.
            // Currnetly, will delete hidden blocks after figuring out neighbors!
            hiddenBlocks.push_back(block_id);

            // hiddenBlocks.push_back(&block);
        }
        else
        {
            visibleBlocks.push_back(&block);
        }
    }

    for (const auto& block_id : block_ids_to_discard)
    {
        blockMap->erase(block_id);
    }

    // TODO: currently, delete hidden blocks to save memory.
    for (const auto& block_id : hiddenBlocks)
    {
        blockMap->erase(block_id);
    }
    hiddenBlocks.clear();
}

const Block* Chunk::getReachableBlock(const Ray& ray) const
{
    const Block* reachable_block = nullptr;

    // Find which block the player can reach.
    // TODO: reduce the visible blocks to check by using the player's reach and position in this chunk instead of
    // evaluating all visible blocks in the chunk.
    float min_dist = std::numeric_limits<float>::infinity();
    for (const auto& block : visibleBlocks)
    {
        float t_min = min_dist; // Can be any value; it will be modified by the intersection test.
        const bool intersected = CollisionHandler::rayShapeIntersect(ray, block->getCollisionShape(), &t_min);

        if (intersected && (t_min < min_dist))
        {
            reachable_block = block;
            min_dist = t_min;
        }
    }

    return reachable_block;
}

glm::vec2 Chunk::getPos() const
{
    return position;
}

std::string Chunk::getPosStr() const
{
    return glm::to_string(position);
}

const std::list<Block*>& Chunk::getVisibleBlocks() const
{
    return visibleBlocks;
}
