#include "chunk.hpp"

#include "engine/physics/collision-handler.hpp"

#include <algorithm>

void Chunk::initContainer()
{
    assert(blocks == nullptr);

    blocks = std::make_unique<BlockContainer>();
    blocks->resize(static_cast<size_t>(size + EDGE_OFFSET * 2));
    for (auto& y : *blocks)
    {
        y.resize(static_cast<size_t>(size + EDGE_OFFSET * 2));
        for (auto& z : y)
        {
            z.resize(static_cast<size_t>(size + EDGE_OFFSET * 2));
        }
    }
}

std::shared_ptr<Block> Chunk::getBlock(const glm::vec3& global_pos) const
{
    const glm::uvec3 local_pos = static_cast<glm::uvec3>(getLocalPos(global_pos));
    return (*blocks)[local_pos.x][local_pos.y][local_pos.z];
}

glm::vec3 Chunk::getLocalPos(const glm::vec3& global_pos) const
{
    return global_pos - (minBounds - glm::vec3(EDGE_OFFSET));
}

void Chunk::generateBlock(const glm::vec3& global_pos, const BlockType type)
{
    const glm::uvec3 local_pos = static_cast<glm::uvec3>(getLocalPos(global_pos));
    (*blocks)[local_pos.x][local_pos.y][local_pos.z] = AVAILABLE_BLOCKS.at(type);
    ++blockCount;
}

void Chunk::resetBlock(const glm::vec3& global_pos)
{
    const glm::uvec3 local_pos = static_cast<glm::uvec3>(getLocalPos(global_pos));
    (*blocks)[local_pos.x][local_pos.y][local_pos.z].reset();
    --blockCount;
}

bool Chunk::doesBlockExist(const glm::vec3& global_pos) const
{
    return getBlock(global_pos) != nullptr;
}

bool Chunk::isBlockVisible(const glm::vec3& global_pos) const
{
    return visibleBlocks.contains(global_pos);
}

bool Chunk::isBlockHidden(const glm::vec3& global_pos) const
{
    const std::vector<glm::vec3> offsets = {
        glm::vec3(0.0, 1.0, 0.0),  // +y
        glm::vec3(0.0, -1.0, 0.0), // -y
        glm::vec3(1.0, 0.0, 0.0),  // +x
        glm::vec3(-1.0, 0.0, 0.0), // -x
        glm::vec3(0.0, 0.0, 1.0),  // +z
        glm::vec3(0.0, 0.0, -1.0), // -z
    };
    for (const auto& offset : offsets)
    {
        const glm::vec3 neighbor = global_pos + offset;
        if (!doesBlockExist(neighbor))
        {
            // There must be a visible face.
            return false;
        }
    }
    return true;
}

bool Chunk::isInChunkBounds(const glm::vec3& global_pos) const
{
    const bool in_x_bounds = (global_pos.x >= minBounds.x) && (global_pos.x <= maxBounds.x);
    const bool in_y_bounds = (global_pos.y >= minBounds.y) && (global_pos.y <= maxBounds.y);
    const bool in_z_bounds = (global_pos.z >= minBounds.z) && (global_pos.z <= maxBounds.z);
    return in_x_bounds && in_y_bounds && in_z_bounds;
}

bool Chunk::isInEdgeBounds(const glm::vec3& global_pos) const
{
    const glm::vec3 min_bounds = minBounds - glm::vec3(static_cast<float>(EDGE_OFFSET));
    const glm::vec3 max_bounds = maxBounds + glm::vec3(static_cast<float>(EDGE_OFFSET));
    const bool in_x_bounds = (global_pos.x >= min_bounds.x) && (global_pos.x <= max_bounds.x);
    const bool in_y_bounds = (global_pos.y >= min_bounds.y) && (global_pos.y <= max_bounds.y);
    const bool in_z_bounds = (global_pos.z >= min_bounds.z) && (global_pos.z <= max_bounds.z);
    return in_x_bounds && in_y_bounds && in_z_bounds;
}

void Chunk::generateMesh()
{
    std::vector<Model::Vertex> vertices;
    std::vector<Model::Index> indices;

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
            // NOTE: assume `visibleBlocks` will never be edge blocks, so neighbor is never out of bounds.
            const bool neighbor_not_exist = !doesBlockExist(neighbor);
            if (neighbor_not_exist)
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

                const glm::vec3 color = getBlock(block_pos)->color;
                const glm::vec3 face_pos = block_pos + offset_to_face;
                for (unsigned j = 0; j < 4; ++j)
                {
                    vertices.emplace_back(Model::Vertex((face_pos + v_offsets[j]), normal, color, uvs[j]));
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

    model = Model(vertices, indices);
}

Chunk::Chunk(const FastNoiseLite& height_noise, const glm::vec3& center_pos, const int size)
    : center(center_pos), size(size)
{
    const float half_size = (static_cast<float>(size) * 0.5f);

    minBounds = center_pos - glm::vec3(half_size);
    maxBounds = center_pos + glm::vec3(half_size - 1.0f);

    const int x_start = static_cast<int>(minBounds.x);
    const int y_start = static_cast<int>(minBounds.y);
    const int z_start = static_cast<int>(minBounds.z);
    const int x_end = static_cast<int>(maxBounds.x);
    const int y_end = static_cast<int>(maxBounds.y);
    const int z_end = static_cast<int>(maxBounds.z);

    // Start at the corner of the chunk and offset it by -`NUM_EDGE_BLOCKS` to figure out the blocks in neighboring
    // chunks. End at the corner with an offset of +`NUM_EDGE_BLOCKS` or a condition `<=` to reach blocks in neighboring
    // chunks.

    // Set up block container.
    initContainer();

    // Generate blocks.
    for (int z = z_start - EDGE_OFFSET; z <= z_end + EDGE_OFFSET; ++z)
    {
        for (int x = x_start - EDGE_OFFSET; x <= x_end + EDGE_OFFSET; ++x)
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

                // Number of blocks above the max block of this chunk.
                const int num_blocks_above = global_height - y_end;
                assert(num_blocks_above >= 0);
                height += std::min(EDGE_OFFSET, num_blocks_above);
            }

            for (int y = y_start - EDGE_OFFSET; y <= height; ++y)
            {
                glm::vec3 block_pos(x, y, z);

                BlockType block_type = BlockType::GRASS;
                if (y < global_height - 3)
                {
                    block_type = BlockType::STONE;
                }
                else if (y <= SEA_LEVEL && global_height <= SEA_LEVEL)
                {
                    block_type = BlockType::SAND;
                }
                else if (y < global_height)
                {
                    block_type = BlockType::DIRT;
                }

                generateBlock(block_pos, block_type);
            }
        }
    }

    // Remove the container if there aren't any blocks; to save memory.
    if (blockCount == 0)
    {
        blocks.reset();
        return;
    }

    // Determine the visible blocks; ignore edge blocks.
    for (int x = x_start; x <= x_end; ++x)
    {
        for (int y = y_start; y <= y_end; ++y)
        {
            for (int z = z_start; z <= z_end; ++z)
            {
                const glm::vec3 global_pos(x, y, z);
                if (doesBlockExist(global_pos) && !isBlockHidden(global_pos))
                {
                    visibleBlocks.emplace(global_pos);
                }
            }
        }
    }

    generateMesh();
}

void Chunk::addBlock(const glm::vec3& global_pos)
{
    // Ignore if it isn't in edge bounds, or if it already exist in a non-empty chunk.
    if (!isInEdgeBounds(global_pos) || ((blockCount > 0) && doesBlockExist(global_pos)))
    {
        return;
    }

    // Initialize the container if this chunk was previously empty.
    if (blockCount == 0)
    {
        initContainer();
    }

    // Add the block.
    generateBlock(global_pos, BlockType::DEFAULT);

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
        const glm::vec3 neighbor = global_pos + offset;
        if (isInChunkBounds(neighbor) && isBlockVisible(neighbor) && isBlockHidden(neighbor))
        {
            visibleBlocks.erase(neighbor);
        }
    }

    // Add it to visible if it's within chunk bounds and is not enclosed.
    if (isInChunkBounds(global_pos) && !isBlockHidden(global_pos))
    {
        visibleBlocks.emplace(global_pos);
    }

    // TODO: utilize block pos to redo mesh.
    generateMesh();
}

void Chunk::removeBlock(const glm::vec3& global_pos)
{
    // Ignore if it doesn't exist in this chunk.
    if (!isInEdgeBounds(global_pos) || (blockCount <= 0) || !doesBlockExist(global_pos))
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
        const glm::vec3 neighbor = global_pos + offset;
        // Check (1) not a edge block, (2) already exists, and (3) is not visible.
        if (isInChunkBounds(neighbor) && doesBlockExist(neighbor) && !visibleBlocks.contains(neighbor))
        {
            visibleBlocks.emplace(neighbor);
        }
    }

    // Delete the block.
    visibleBlocks.erase(global_pos);
    resetBlock(global_pos);

    // Delete the container if there are no more blocks in this chunk.
    if (blockCount == 0)
    {
        blocks.reset();
    }

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
        const bool intersected = CollisionHandler::rayToShapeIntersect(
            ray,
            Aabb3d(glm::vec3(block_pos) - glm::vec3(0.5f), glm::vec3(block_pos) + glm::vec3(0.5f)),
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

bool Chunk::doesEntityIntersect(
    const glm::vec3& velocity,
    const float delta,
    const Aabb3d& hitbox,
    float& new_delta,
    glm::vec3* normal) const
{
    new_delta = delta;

    bool intersected = false;
    glm::vec3 closest_normal{};
    const Aabb3d broad_phase_aabb = CollisionHandler::getBroadPhaseAabb(hitbox, velocity, delta);

    for (const auto& block_pos : visibleBlocks)
    {
        const Aabb3d block_hitbox(glm::vec3(block_pos) - glm::vec3(0.5f), glm::vec3(block_pos) + glm::vec3(0.5f));

        // Check whether to ignore the current block given the broad phase AABB.
        if (!CollisionHandler::shapeToShapeIntersect(broad_phase_aabb, block_hitbox))
        {
            continue;
        }

        glm::vec3 curr_normal{};
        const float t_min = CollisionHandler::sweptAabbPerAxis(hitbox, block_hitbox, velocity, delta, curr_normal);

        assert(t_min >= 0.0f);
        if (t_min != std::numeric_limits<float>::infinity())
        {
            // Get the minimum because it represents the closest collision.
            if (t_min < new_delta)
            {
                new_delta = t_min;
                closest_normal = curr_normal;
            }
            intersected = true;
        }
    }

    // Update modifiable parameters.
    if (normal != nullptr)
    {
        *normal = closest_normal;
    }

    return intersected;
}

ChunkCenter Chunk::getCenter() const
{
    return center;
}

const Model& Chunk::getModel() const
{
    return model;
}
