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
    std::unordered_set<glm::vec3> block_pos_to_discard;

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
                blockMap->emplace(position, position);

                const bool is_z_edge = (z == z_start - 1) || (z == z_start + size);
                const bool is_x_edge = (x == x_start - 1) || (x == x_start + size);
                if (is_z_edge || is_x_edge)
                {
                    block_pos_to_discard.emplace(position);
                }
            }
        }
    }

    for (auto& map : *blockMap)
    {
        const glm::vec3& block_pos = map.first;
        if (block_pos_to_discard.contains(block_pos))
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
            //  TODO: potential idea to save memory is to generate the blocks after the player
            //  modifies the world (place/break blocks).
            hiddenBlocks.push_back(block_pos);
        }
        else
        {
            visibleBlocks.emplace(block_pos);
        }
    }

    for (const auto& block_pos : block_pos_to_discard)
    {
        blockMap->erase(block_pos);
    }
}

void Chunk::removeBlock(const glm::vec3 block_pos)
{
    // Add new visible blocks.
    const glm::vec3 pos_y = block_pos + glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::vec3 neg_y = block_pos + glm::vec3(0.0f, -1.0f, 0.0f);
    const glm::vec3 pos_x = block_pos + glm::vec3(1.0f, 0.0f, 0.0f);
    const glm::vec3 neg_x = block_pos + glm::vec3(-1.0f, 0.0f, 0.0f);
    const glm::vec3 pos_z = block_pos + glm::vec3(0.0f, 0.0f, 1.0f);
    const glm::vec3 neg_z = block_pos + glm::vec3(0.0f, 0.0f, -1.0f);
    if ((*blockMap).contains(pos_y))
    {
        visibleBlocks.emplace(pos_y);
    }
    if ((*blockMap).contains(neg_y))
    {
        visibleBlocks.emplace(neg_y);
    }
    if ((*blockMap).contains(pos_x))
    {
        visibleBlocks.emplace(pos_x);
    }
    if ((*blockMap).contains(neg_x))
    {
        visibleBlocks.emplace(neg_x);
    }
    if ((*blockMap).contains(pos_z))
    {
        visibleBlocks.emplace(pos_z);
    }
    if ((*blockMap).contains(neg_z))
    {
        visibleBlocks.emplace(neg_z);
    }

    // Delete the block.
    visibleBlocks.erase(block_pos);
    blockMap->erase(block_pos);
}

const std::optional<glm::vec3> Chunk::getReachableBlock(const Ray& ray) const
{
    std::optional<glm::vec3> reachable_block_pos;

    // Find which block the player can reach.
    // TODO: reduce the visible blocks to check by using the player's reach and position in this chunk instead of
    // evaluating all visible blocks in the chunk.
    float min_dist = std::numeric_limits<float>::infinity();
    for (const auto& block_pos : visibleBlocks)
    {
        float t_min = min_dist; // Can be any value; it will be modified by the intersection test.
        const bool intersected =
            CollisionHandler::rayShapeIntersect(ray, ((*blockMap)[block_pos]).getCollisionShape(), &t_min);

        if (intersected && (t_min < min_dist))
        {
            reachable_block_pos = block_pos;
            min_dist = t_min;
        }
    }

    return reachable_block_pos;
}

glm::vec2 Chunk::getPos() const
{
    return position;
}

std::string Chunk::getPosStr() const
{
    return glm::to_string(position);
}

const std::list<glm::vec3> Chunk::getVisibleBlockPositions() const
{
    std::list<glm::vec3> positions;
    for (const auto& block_pos : visibleBlocks)
    {
        positions.push_back(((*blockMap)[block_pos]).position);
    }
    return positions;
}
