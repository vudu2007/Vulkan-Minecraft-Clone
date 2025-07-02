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

void Chunk::generateMesh()
{
    vertices.clear();
    indices.clear();

    // Iterate through the visible blocks to generate visible faces.
    for (const auto& block_pos : visibleBlocks)
    {
        // Find the visible faces and generate vertices for the faces.
        unsigned face_index = 0;
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

            // Determine if this face should be visible and generate it.
            const bool neighbor_exist = (*blockMap).contains(neighbor);
            if (!neighbor_exist)
            {
                const glm::vec3 offset_to_face = offset / 2.0f; // Face is inbetween current and neighbor.
                glm::vec3 v_0_offset;
                glm::vec3 v_1_offset;
                glm::vec3 v_2_offset;
                glm::vec3 v_3_offset;
                glm::vec3 normal;

                // Generating counter-clockwise.
                switch (face_index)
                {
                case 0: { // +y
                    v_0_offset = glm::vec3(-0.5f, 0.0f, -0.5f);
                    v_1_offset = glm::vec3(-0.5f, 0.0f, 0.5f);
                    v_2_offset = glm::vec3(0.5f, 0.0f, 0.5f);
                    v_3_offset = glm::vec3(0.5f, 0.0f, -0.5f);
                    normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    break;
                }
                case 1: { // -y
                    v_0_offset = glm::vec3(-0.5f, 0.0f, -0.5f);
                    v_1_offset = glm::vec3(0.5f, 0.0f, -0.5f);
                    v_2_offset = glm::vec3(0.5f, 0.0f, 0.5f);
                    v_3_offset = glm::vec3(-0.5f, 0.0f, 0.5f);
                    normal = glm::vec3(0.0f, -1.0f, 0.0f);
                    break;
                }
                case 2: { // +x
                    v_0_offset = glm::vec3(0.0f, 0.5f, -0.5f);
                    v_1_offset = glm::vec3(0.0f, 0.5f, 0.5f);
                    v_2_offset = glm::vec3(0.0f, -0.5f, 0.5f);
                    v_3_offset = glm::vec3(0.0f, -0.5f, -0.5f);
                    normal = glm::vec3(1.0f, 0.0f, 0.0f);
                    break;
                }
                case 3: { // -x
                    v_0_offset = glm::vec3(0.0f, 0.5f, -0.5f);
                    v_1_offset = glm::vec3(0.0f, -0.5f, -0.5f);
                    v_2_offset = glm::vec3(0.0f, -0.5f, 0.5f);
                    v_3_offset = glm::vec3(0.0f, 0.5f, 0.5f);
                    normal = glm::vec3(-1.0f, 0.0f, 0.0f);
                    break;
                }
                case 4: { // +z
                    v_0_offset = glm::vec3(-0.5f, 0.5f, 0.0f);
                    v_1_offset = glm::vec3(-0.5f, -0.5f, 0.0f);
                    v_2_offset = glm::vec3(0.5f, -0.5f, 0.0f);
                    v_3_offset = glm::vec3(0.5f, 0.5f, 0.0f);
                    normal = glm::vec3(0.0f, 0.0f, 1.0f);
                    break;
                }
                case 5: { // -z
                    v_0_offset = glm::vec3(-0.5f, 0.5f, 0.0f);
                    v_1_offset = glm::vec3(0.5f, 0.5f, 0.0f);
                    v_2_offset = glm::vec3(0.5f, -0.5f, 0.0f);
                    v_3_offset = glm::vec3(-0.5f, -0.5f, 0.0f);
                    normal = glm::vec3(0.0f, 0.0f, -1.0f);
                    break;
                }
                }

                const glm::vec3 color = (*blockMap)[block_pos].color;
                const glm::vec3 face_pos = block_pos + offset_to_face;
                vertices.emplace_back(face_pos + v_0_offset, normal, color, glm::vec2(0.0f, 0.0f));
                vertices.emplace_back(face_pos + v_1_offset, normal, color, glm::vec2(1.0f, 0.0f));
                vertices.emplace_back(face_pos + v_2_offset, normal, color, glm::vec2(1.0f, 1.0f));
                vertices.emplace_back(face_pos + v_3_offset, normal, color, glm::vec2(0.0f, 1.0f));
            }

            ++face_index;
        }
    }

    // Vertices are in a specific order so indicies will be in increasing order.
    for (size_t i = 0; i < vertices.size(); i += 4)
    {
        indices.emplace_back(i);
        indices.emplace_back(i + 1);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 3);
        indices.emplace_back(i);
    }
}

Chunk::Chunk(const FastNoiseLite& height_noise, const glm::vec2& center_pos, const int size)
    : position(center_pos), size(size)
{
    const glm::vec3 color_grass(0.349f, 0.651f, 0.290f);
    const glm::vec3 color_dirt(0.396f, 0.263f, 0.129f);
    const glm::vec3 color_stone(0.439f, 0.502f, 0.565f);
    const glm::vec3 color_sand(0.96f, 0.87f, 0.70f);

    const float half_size = (size / static_cast<float>(2));

    const int x_start = static_cast<int>(center_pos.x - half_size);
    const int z_start = static_cast<int>(center_pos.y - half_size);

    x_bounds = glm::vec2(center_pos.x - half_size, center_pos.x + half_size);
    z_bounds = glm::vec2(center_pos.y - half_size, center_pos.y + half_size);

    const int SEA_LEVEL = 0;
    const int MAX_HEIGHT = 50;
    int min_height = -50; // std::numeric_limits<int>::max(); // TODO
    const int HEIGHT_RANGE = MAX_HEIGHT - min_height;
    assert(HEIGHT_RANGE > 0);

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
            const float noise_val = (height_noise.GetNoise(static_cast<float>(x), static_cast<float>(z)) + 1.0f) * 0.5f;
            int height = static_cast<int>(std::floorf(noise_val * HEIGHT_RANGE));
            if (min_height < 0)
            {
                height += min_height;
            }

            min_height = std::min(min_height, height);

            for (int y = min_height; y <= height; ++y)
            {
                const glm::vec3 position(static_cast<float>(x), y, static_cast<float>(z));

                glm::vec3 block_color = color_grass;
                if (y < height - 3)
                {
                    block_color = color_stone;
                }
                else if (y <= SEA_LEVEL && height <= SEA_LEVEL)
                {
                    block_color = color_sand;
                }
                else if (y < height)
                {
                    block_color = color_dirt;
                }

                blockMap->emplace(position, Block(position, block_color));

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

    generateMesh();
}

void Chunk::addBlock(const glm::vec3 block_pos)
{
    // Ignore if it isn't in edge bounds or if it already exist.
    if (!checkInEdgeBounds(block_pos) || blockMap->contains(block_pos))
    {
        return;
    }

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

    // TODO: utilize block pos to redo mesh.
    generateMesh();
}

void Chunk::removeBlock(const glm::vec3 block_pos)
{
    // Ignore if it doesn't exist in this chunk.
    if (!(*blockMap).contains(block_pos))
    {
        return;
    }

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

    // TODO: utilize block pos to redo mesh.
    generateMesh();
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

const std::vector<Model::Vertex> Chunk::getVertices() const
{
    return vertices;
}

const std::vector<Model::Index> Chunk::getIndices() const
{
    return indices;
}
