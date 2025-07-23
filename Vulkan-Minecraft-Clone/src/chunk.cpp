#include "chunk.hpp"

#include "engine/physics/collision-handler.hpp"

#include <algorithm>
#include <array>

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
    const bool in_x_bounds = (block_pos.x >= xBounds[0]) && (block_pos.x <= xBounds[1]);
    const bool in_y_bounds = (block_pos.y >= yBounds[0]) && (block_pos.y <= yBounds[1]);
    const bool in_z_bounds = (block_pos.z >= zBounds[0]) && (block_pos.z <= zBounds[1]);
    return in_x_bounds && in_y_bounds && in_z_bounds;
}

bool Chunk::checkInEdgeBounds(const glm::vec3& block_pos) const
{
    const bool in_x_bounds =
        (block_pos.x >= xBounds[0] - NUM_EDGE_BLOCKS) && (block_pos.x <= xBounds[1] + NUM_EDGE_BLOCKS);
    const bool in_y_bounds =
        (block_pos.y >= yBounds[0] - NUM_EDGE_BLOCKS) && (block_pos.y <= yBounds[1] + NUM_EDGE_BLOCKS);
    const bool in_z_bounds =
        (block_pos.z >= zBounds[0] - NUM_EDGE_BLOCKS) && (block_pos.z <= zBounds[1] + NUM_EDGE_BLOCKS);
    return in_x_bounds && in_y_bounds && in_z_bounds;
}

void Chunk::generateMesh()
{
    vertices.clear();
    indices.clear();

    // Iterate through the visible blocks to generate visible faces.
    constexpr std::array<glm::vec3, 6> offsets = {
        glm::vec3(0.0f, 1.0f, 0.0f),  // +y
        glm::vec3(0.0f, -1.0f, 0.0f), // -y
        glm::vec3(1.0f, 0.0f, 0.0f),  // +x
        glm::vec3(-1.0f, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, 0.0f, 1.0f),  // +z
        glm::vec3(0.0f, 0.0f, -1.0f), // -z
    };
    constexpr std::array<glm::vec2, 4> uvs = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
    };
    for (const auto& block_pos : visibleBlocks)
    {
        // Find the visible faces and generate vertices for the faces.
        for (unsigned i = 0; i < 6; ++i)
        {
            const glm::vec3& offset = offsets[i];
            const glm::vec3 neighbor = block_pos + offset;

            // Determine if this face should be visible and generate it.
            const bool neighbor_exist = (*blockMap).contains(neighbor);
            if (!neighbor_exist)
            {
                const glm::vec3 offset_to_face = offset / 2.0f; // Face is inbetween current and neighbor.
                std::array<glm::vec3, 4> v_offsets{};
                glm::vec3 normal;

                // Generating counter-clockwise.
                switch (i)
                {
                case 0: { // +y
                    v_offsets[0] = glm::vec3(-0.5f, 0.0f, -0.5f);
                    v_offsets[1] = glm::vec3(-0.5f, 0.0f, 0.5f);
                    v_offsets[2] = glm::vec3(0.5f, 0.0f, 0.5f);
                    v_offsets[3] = glm::vec3(0.5f, 0.0f, -0.5f);
                    normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    break;
                }
                case 1: { // -y
                    v_offsets[0] = glm::vec3(-0.5f, 0.0f, -0.5f);
                    v_offsets[1] = glm::vec3(0.5f, 0.0f, -0.5f);
                    v_offsets[2] = glm::vec3(0.5f, 0.0f, 0.5f);
                    v_offsets[3] = glm::vec3(-0.5f, 0.0f, 0.5f);
                    normal = glm::vec3(0.0f, -1.0f, 0.0f);
                    break;
                }
                case 2: { // +x
                    v_offsets[0] = glm::vec3(0.0f, 0.5f, -0.5f);
                    v_offsets[1] = glm::vec3(0.0f, 0.5f, 0.5f);
                    v_offsets[2] = glm::vec3(0.0f, -0.5f, 0.5f);
                    v_offsets[3] = glm::vec3(0.0f, -0.5f, -0.5f);
                    normal = glm::vec3(1.0f, 0.0f, 0.0f);
                    break;
                }
                case 3: { // -x
                    v_offsets[0] = glm::vec3(0.0f, 0.5f, -0.5f);
                    v_offsets[1] = glm::vec3(0.0f, -0.5f, -0.5f);
                    v_offsets[2] = glm::vec3(0.0f, -0.5f, 0.5f);
                    v_offsets[3] = glm::vec3(0.0f, 0.5f, 0.5f);
                    normal = glm::vec3(-1.0f, 0.0f, 0.0f);
                    break;
                }
                case 4: { // +z
                    v_offsets[0] = glm::vec3(-0.5f, 0.5f, 0.0f);
                    v_offsets[1] = glm::vec3(-0.5f, -0.5f, 0.0f);
                    v_offsets[2] = glm::vec3(0.5f, -0.5f, 0.0f);
                    v_offsets[3] = glm::vec3(0.5f, 0.5f, 0.0f);
                    normal = glm::vec3(0.0f, 0.0f, 1.0f);
                    break;
                }
                case 5: { // -z
                    v_offsets[0] = glm::vec3(-0.5f, 0.5f, 0.0f);
                    v_offsets[1] = glm::vec3(0.5f, 0.5f, 0.0f);
                    v_offsets[2] = glm::vec3(0.5f, -0.5f, 0.0f);
                    v_offsets[3] = glm::vec3(-0.5f, -0.5f, 0.0f);
                    normal = glm::vec3(0.0f, 0.0f, -1.0f);
                    break;
                }
                }

                const glm::vec3 color = (*blockMap)[block_pos].color;
                const glm::vec3 face_pos = block_pos + offset_to_face;
                for (unsigned j = 0; j < 4; ++j)
                {
                    vertices.emplace_back((face_pos + v_offsets[j]), normal, color, uvs[j]);
                }
            }
        }
    }

    // Vertices are in a specific order so indices will be in increasing order.
    for (Model::Index i = 0; i < static_cast<Model::Index>(vertices.size()); i += 4)
    {
        indices.emplace_back(i);
        indices.emplace_back(i + 1);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 3);
        indices.emplace_back(i);
    }
}

Chunk::Chunk(const FastNoiseLite& height_noise, const glm::vec3& center_pos, const int size)
    : center(center_pos), size(size)
{
    const float half_size = (static_cast<float>(size) / 2.0f);

    xBounds = glm::vec2(center_pos.x - half_size, center_pos.x + half_size - 1);
    yBounds = glm::vec2(center_pos.y - half_size, center_pos.y + half_size - 1);
    zBounds = glm::vec2(center_pos.z - half_size, center_pos.z + half_size - 1);

    const int x_start = static_cast<int>(xBounds[0]);
    const int y_start = static_cast<int>(yBounds[0]);
    const int z_start = static_cast<int>(zBounds[0]);
    const int x_end = static_cast<int>(xBounds[1]);
    const int y_end = static_cast<int>(yBounds[1]);
    const int z_end = static_cast<int>(zBounds[1]);

    // Start at the corner of the chunk and offset it by -`NUM_EDGE_BLOCKS` to figure out the blocks in neighboring
    // chunks. End at the corner with an offset of +`NUM_EDGE_BLOCKS` or a condition `<=` to reach blocks in neighboring
    // chunks.
    blockMap = std::make_unique<std::unordered_map<glm::vec3, Block>>();
    for (int z = z_start - NUM_EDGE_BLOCKS; z <= z_end + NUM_EDGE_BLOCKS; ++z)
    {
        for (int x = x_start - NUM_EDGE_BLOCKS; x <= x_end + NUM_EDGE_BLOCKS; ++x)
        {
            // At this moment, height is the global height; tallest point at this xz-position.
            const float noise_val = (height_noise.GetNoise(static_cast<float>(x), static_cast<float>(z)) + 1.0f) * 0.5f;
            const int global_height = static_cast<int>(std::floor(noise_val * HEIGHT_RANGE)) + HEIGHT_OFFSET;

            // Make sure the height is within the chunk and that it exist.
            int height = std::min(global_height, y_end);
            if (height == y_end)
            {
                // We are at the highest block of this chunk so we need to make sure to add any edge blocks that exist
                // in the neighboring chunk above.
                const int num_blocks_above =
                    global_height - y_end; // Number of blocks above the max block of this chunk.
                assert(num_blocks_above >= 0);
                height += std::min(NUM_EDGE_BLOCKS, num_blocks_above);
            }

            for (int y = y_start - NUM_EDGE_BLOCKS; y <= height; ++y)
            {
                const glm::vec3 block_pos(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

                glm::vec3 block_color = COLOR_GRASS;
                if (y < global_height - 3)
                {
                    block_color = COLOR_STONE;
                }
                else if (y <= SEA_LEVEL && global_height <= SEA_LEVEL)
                {
                    block_color = COLOR_SAND;
                }
                else if (y < global_height)
                {
                    block_color = COLOR_DIRT;
                }

                blockMap->emplace(block_pos, Block(block_pos, block_color));
            }
        }
    }

    // Determine the visible blocks.
    for (auto& map : *blockMap)
    {
        const glm::vec3& block_pos = map.first;

        // Ignore if an edge block.
        if (!checkInChunkBounds(block_pos) && checkInEdgeBounds(block_pos))
        {
            continue;
        }

        const Block& block = map.second;
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
    blockMap->emplace(block_pos, Block(block_pos, glm::vec3(1.0f)));

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

const std::vector<Model::Vertex> Chunk::getVertices() const
{
    return vertices;
}

const std::vector<Model::Index> Chunk::getIndices() const
{
    return indices;
}
