#include "chunk.hpp"

#include "engine/physics/collision-handler.hpp"

#include <algorithm>
#include <unordered_set>

bool Chunk::checkBlockHidden(const glm::vec3& block_pos) const
{
    const std::vector<glm::vec3> offsets = {
        glm::vec3(0.0f, 1.0f, 0.0f),  // +y
        glm::vec3(0.0f, -1.0f, 0.0f), // -y
        glm::vec3(1.0f, 0.0f, 0.0f),  // +x
        glm::vec3(-1.0f, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, 0.0f, 1.0f),  // +z
        glm::vec3(0.0f, 0.0f, -1.0f), // -z
    };
    for (const auto& offset : offsets)
    {
        if (!blockMap->contains(block_pos + offset))
        {
            // There must be a visible face.
            return false;
        }
    }
    return true;
}

bool Chunk::checkInChunkBounds(const glm::vec3& block_pos) const
{
    const bool in_x_bounds = (block_pos.x >= x_bounds[0]) && (block_pos.x <= x_bounds[1]);
    const bool in_z_bounds = (block_pos.z >= z_bounds[0]) && (block_pos.z <= z_bounds[1]);
    return in_x_bounds && in_z_bounds;
}

bool Chunk::checkInEdgeBounds(const glm::vec3& block_pos) const
{
    const bool in_x_bounds = (block_pos.x >= x_bounds[0] - 2) && (block_pos.x <= x_bounds[1] + 2);
    const bool in_z_bounds = (block_pos.z >= z_bounds[0] - 2) && (block_pos.z <= z_bounds[1] + 2);
    return in_x_bounds && in_z_bounds;
}

Chunk::Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size) : position(center_pos), size(size)
{
    const float half_size = (size / static_cast<float>(2));

    const int x_start = static_cast<int>(center_pos.x - half_size);
    const int z_start = static_cast<int>(center_pos.y - half_size);

    x_bounds = glm::vec2(center_pos.x - half_size, center_pos.x + half_size);
    z_bounds = glm::vec2(center_pos.y - half_size, center_pos.y + half_size);

    int min_height = -100; // std::numeric_limits<int>::max(); // TODO

    // Keep track of blocks in neighboring chunks to discard later because they shouldn't exist in this chunk.
    // Will use these blocks to figure out whether blocks on edge of this chunk should be visible.
    std::unordered_set<glm::vec3> edge_blocks;

    // Start at the corner of the chunk and offset it by -1 to figure out the blocks in neighboring chunks.
    // End at the corner with an offset of +1 or a condition `<=` to reach blocks in neighboring chunks.
    blockMap = std::make_unique<std::unordered_map<glm::vec3, Block>>();
    for (int z = z_start - 2; z < z_start + size + 2; ++z)
    {
        for (int x = x_start - 2; x < x_start + size + 2; ++x)
        {
            int max_height = static_cast<int>(
                std::floorf(noise.getFractal2D(static_cast<float>(x), static_cast<float>(z), 5, 20.0f, 0.0035f)));

            min_height = std::min(min_height, max_height);

            for (int y = min_height; y <= max_height; ++y)
            {
                const glm::vec3 position(static_cast<float>(x), y, static_cast<float>(z));
                blockMap->emplace(position, position);

                const bool is_z_edge =
                    (z == z_start - 2) || (z == z_start - 1) || (z == z_start + size) || (z == z_start + size + 1);
                const bool is_x_edge =
                    (x == x_start - 2) || (x == x_start - 1) || (x == x_start + size) || (x == x_start + size + 1);
                if (is_z_edge || is_x_edge)
                {
                    edge_blocks.emplace(position);
                }
            }
        }
    }

    for (auto& map : *blockMap)
    {
        const glm::vec3& block_pos = map.first;
        if (edge_blocks.contains(block_pos))
        {
            continue;
        }

        Block& block = map.second;
        if (!checkBlockHidden(block.position))
        {
            visibleBlocks.emplace(block_pos);
        }
    }
}

void Chunk::addBlock(const glm::vec3 block_pos)
{
    // Ignore if it isn't in edge bounds or if it already exist.
    if (checkInEdgeBounds(block_pos) && !blockMap->contains(block_pos))
    {
        // Add the block.
        blockMap->emplace(block_pos, block_pos);

        // Update neighboring blocks.
        // Check if the neighbors are now fully enclosed.
        const std::vector<glm::vec3> offsets = {
            glm::vec3(0.0f, 1.0f, 0.0f),  // +y
            glm::vec3(0.0f, -1.0f, 0.0f), // -y
            glm::vec3(1.0f, 0.0f, 0.0f),  // +x
            glm::vec3(-1.0f, 0.0f, 0.0f), // -x
            glm::vec3(0.0f, 0.0f, 1.0f),  // +z
            glm::vec3(0.0f, 0.0f, -1.0f), // -z
        };
        for (const auto& offset : offsets)
        {
            const glm::vec3 neighbor = block_pos + offset;
            if ((*blockMap).contains(neighbor) && checkBlockHidden(neighbor))
            {
                visibleBlocks.erase(neighbor);
            }
        }

        // Add it to visible if it's within chunk bounds and is not enclosed.
        if (checkInChunkBounds(block_pos) && !checkBlockHidden(block_pos))
        {
            visibleBlocks.emplace(block_pos);
        }
    }
}

void Chunk::removeBlock(const glm::vec3 block_pos)
{
    // Add new visible blocks.
    const std::vector<glm::vec3> offsets = {
        glm::vec3(0.0f, 1.0f, 0.0f),  // +y
        glm::vec3(0.0f, -1.0f, 0.0f), // -y
        glm::vec3(1.0f, 0.0f, 0.0f),  // +x
        glm::vec3(-1.0f, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, 0.0f, 1.0f),  // +z
        glm::vec3(0.0f, 0.0f, -1.0f), // -z
    };
    for (const auto& offset : offsets)
    {
        const glm::vec3 neighbor = block_pos + offset;
        // Check (1) already exists, (2) is not visible, and (3) not a neighboring edge block from a different chunk.
        if ((*blockMap).contains(neighbor) && !visibleBlocks.contains(neighbor) && checkInChunkBounds(neighbor))
        {
            visibleBlocks.emplace(neighbor);
        }
    }

    // Delete the block.
    visibleBlocks.erase(block_pos);
    blockMap->erase(block_pos);
}

const std::optional<glm::vec3> Chunk::getReachableBlock(const Ray& ray, glm::ivec3* face_entered) const
{
    if (face_entered != nullptr)
    {
        *face_entered = glm::ivec3(0);
    }

    std::optional<glm::vec3> reachable_block_pos;

    // Find which block the player can reach.
    // TODO: reduce the visible blocks to check by using the player's reach and position in this chunk instead of
    // evaluating all visible blocks in the chunk.
    float min_dist = std::numeric_limits<float>::infinity();
    for (const auto& block_pos : visibleBlocks)
    {
        float t_min = min_dist; // Can be any value; it will be modified by the intersection test.
        glm::ivec3 curr_face_entered{};
        const bool intersected = CollisionHandler::rayShapeIntersect(
            ray,
            ((*blockMap)[block_pos]).getCollisionShape(),
            &t_min,
            nullptr,
            &curr_face_entered);

        if (intersected && (t_min < min_dist))
        {
            reachable_block_pos = block_pos;
            min_dist = t_min;
            if (face_entered != nullptr)
            {
                *face_entered = curr_face_entered;
            }
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
