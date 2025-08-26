#include "chunk.hpp"

#include "engine/physics/collision-handler.hpp"

#include <algorithm>

void Chunk::initContainer()
{
    assert(blocks == nullptr);

    blocks = std::make_unique<BlockContainer>();

    // `size` cubed.
    size_t num_blocks = static_cast<size_t>(size);
    num_blocks *= num_blocks;
    num_blocks *= num_blocks;

    blocks->resize(num_blocks, BlockType::EMPTY);
}

void Chunk::init()
{
    blockCount = 0;

    const glm::ivec3 start = static_cast<glm::ivec3>(minBounds);
    const glm::ivec3 end = static_cast<glm::ivec3>(maxBounds);

    // Start at the corner of the chunk and offset it by -`NUM_EDGE_BLOCKS` to figure out the blocks in neighboring
    // chunks. End at the corner with an offset of +`NUM_EDGE_BLOCKS` or a condition `<=` to reach blocks in neighboring
    // chunks.

    // Set up block container.
    initContainer();

    // Generate blocks.
    std::unordered_set<glm::vec3> neighboring_chunk_blocks;
    constexpr int neighbor_chunk_offset = 2;
    for (int z = start.z - neighbor_chunk_offset; z <= end.z + neighbor_chunk_offset; ++z)
    {
        for (int x = start.x - neighbor_chunk_offset; x <= end.x + neighbor_chunk_offset; ++x)
        {
            // At this moment, height is the global height; tallest point at this xz-position.
            const float noise_val = (heightNoise.GetNoise(static_cast<float>(x), static_cast<float>(z)) + 1.0f) * 0.5f;
            const int global_height = static_cast<int>(std::floor(noise_val * HEIGHT_RANGE)) + HEIGHT_OFFSET;

            // Make sure the height is within the chunk and that it exist.
            int height = std::min(global_height, end.y);
            if (height == end.y)
            {
                // We are at the highest block of this chunk so we need to make sure to add any edge blocks that exist
                // in the neighboring chunk above.

                // Number of blocks above the max block of this chunk.
                const int num_blocks_above = global_height - end.y;
                assert(num_blocks_above >= 0);
                height += std::min(neighbor_chunk_offset, num_blocks_above);
            }

            for (int y = start.y - neighbor_chunk_offset; y <= height; ++y)
            {
                glm::vec3 block_pos(x, y, z);

                BlockType block_type = BlockType::GRASS;
                if (isBlockOnEdge(block_pos))
                {
                    block_type = BlockType::RED;
                }
                else if (y < global_height - 3)
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

                if (!generateBlock(block_pos, block_type))
                {
                    // Block didn't generate, so this must be a block in a neighboring chunk.
                    neighboring_chunk_blocks.emplace(block_pos);
                }
            }
        }
    }

    // Don't initialize the container if there are no blocks.
    if (blockCount == 0)
    {
        blocks.reset();
        return;
    }

    // Determine the visible blocks; ignore neighboring chunk blocks.
    for (int x = start.x; x <= end.x; ++x)
    {
        for (int y = start.y; y <= end.y; ++y)
        {
            for (int z = start.z; z <= end.z; ++z)
            {
                const glm::vec3 global_pos(x, y, z);
                if (isBlockPresent(global_pos) &&
                    (!isBlockHidden(global_pos, neighboring_chunk_blocks) || !isBlockHidden(global_pos)))
                {
                    visibleBlocks.emplace(global_pos);
                }
            }
        }
    }

    // Don't store blocks in the container if there are no visible blocks.
    if (visibleBlocks.empty())
    {
        blocks.reset();
        return;
    }
}

int Chunk::getBlockIndex(const glm::vec3& global_pos) const
{
    const glm::ivec3 local_pos = getLocalPos(global_pos);
    return local_pos.x + (local_pos.y * size) + (local_pos.z * size * size);
}

Chunk::BlockType Chunk::getBlockType(const glm::vec3& global_pos) const
{
    return (*blocks)[getBlockIndex(global_pos)];
}

std::weak_ptr<Block> Chunk::getBlock(const glm::vec3& global_pos) const
{
    return BLOCK_PALETTE.at(getBlockType(global_pos));
}

glm::ivec3 Chunk::getLocalPos(const glm::vec3& global_pos) const
{
    return static_cast<glm::ivec3>(global_pos - minBounds);
}

bool Chunk::generateBlock(const glm::vec3& global_pos, const BlockType type)
{
    if (!isInChunkBounds(global_pos))
    {
        return false;
    }

    (*blocks)[getBlockIndex(global_pos)] = type;
    ++blockCount;
    return true;
}

bool Chunk::resetBlock(const glm::vec3& global_pos)
{
    if (!isInChunkBounds(global_pos))
    {
        return false;
    }

    (*blocks)[getBlockIndex(global_pos)] = BlockType::EMPTY;
    --blockCount;
    return true;
}

bool Chunk::isBlockPresent(const glm::vec3& global_pos) const
{
    return (blockCount > 0) && (isInChunkBounds(global_pos)) &&
           (blocks == nullptr || getBlockType(global_pos) != BlockType::EMPTY);
}

bool Chunk::isBlockVisible(const glm::vec3& global_pos) const
{
    return visibleBlocks.contains(global_pos);
}

bool Chunk::isBlockHidden(const glm::vec3& global_pos) const
{
    const std::vector<glm::vec3> offsets = {
        glm::vec3(1.0, 0.0, 0.0),  // +x
        glm::vec3(0.0, 1.0, 0.0),  // +y
        glm::vec3(0.0, 0.0, 1.0),  // +z
        glm::vec3(-1.0, 0.0, 0.0), // -x
        glm::vec3(0.0, -1.0, 0.0), // -y
        glm::vec3(0.0, 0.0, -1.0), // -z
    };
    for (size_t i = 0; i < offsets.size(); ++i)
    {
        const glm::vec3 neighbor = global_pos + offsets[i];
        if (!isBlockPresent(neighbor) && (neighboringChunks[i] != nullptr) &&
            !neighboringChunks[i]->isBlockPresent(neighbor))
        {
            // There must be a visible face; therefore, a visible block.
            return false;
        }
    }
    return true;
}

bool Chunk::isBlockHidden(const glm::vec3& global_pos, const std::unordered_set<glm::vec3>& neighboring_blocks) const
{
    const std::vector<glm::vec3> offsets = {
        glm::vec3(1.0, 0.0, 0.0),  // +x
        glm::vec3(0.0, 1.0, 0.0),  // +y
        glm::vec3(0.0, 0.0, 1.0),  // +z
        glm::vec3(-1.0, 0.0, 0.0), // -x
        glm::vec3(0.0, -1.0, 0.0), // -y
        glm::vec3(0.0, 0.0, -1.0), // -z
    };
    for (size_t i = 0; i < offsets.size(); ++i)
    {
        const glm::vec3 neighbor = global_pos + offsets[i];
        if (!isBlockPresent(neighbor) && !neighboring_blocks.contains(neighbor))
        {
            // There must be a visible face; therefore, a visible block.
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

Chunk::Chunk(const FastNoiseLite& height_noise, const glm::vec3& center_pos, const int size)
    : center(center_pos), size(size), heightNoise(height_noise)
{
    const float half_size = (static_cast<float>(size) * 0.5f);
    minBounds = center_pos - glm::vec3(half_size);
    maxBounds = center_pos + glm::vec3(half_size - 1.0f);

    init();
}

void Chunk::addBlock(const glm::vec3& global_pos)
{
    // Ignore if it isn't in this chunk, or it already exist in this non-empty chunk.
    if (!isInChunkBounds(global_pos) || (blockCount > 0) && isBlockPresent(global_pos))
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

    // Add it to visible if it's not enclosed.
    if (!isBlockHidden(global_pos))
    {
        visibleBlocks.emplace(global_pos);
    }

    // Update neighboring blocks in this chunk.
    // Check if the neighbors are now fully enclosed.
    const std::vector<glm::vec3> offsets = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // +x
        glm::vec3(0.0f, 1.0f, 0.0f),  // +y
        glm::vec3(0.0f, 0.0f, 1.0f),  // +z
        glm::vec3(-1.0f, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, -1.0f, 0.0f), // -y
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
}

void Chunk::removeBlock(const glm::vec3& global_pos)
{
    // Ignore if it doesn't exist in this chunk, the chunk is empty, or the block does not exist.
    if (!isInChunkBounds(global_pos) || (blockCount <= 0) || !isBlockPresent(global_pos))
    {
        return;
    }

    // Delete the block.
    visibleBlocks.erase(global_pos);
    resetBlock(global_pos);

    // Add new visible neighbor blocks.
    const std::vector<glm::vec3> offsets = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // +x
        glm::vec3(0.0f, 1.0f, 0.0f),  // +y
        glm::vec3(0.0f, 0.0f, 1.0f),  // +z
        glm::vec3(-1.0f, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, -1.0f, 0.0f), // -y
        glm::vec3(0.0f, 0.0f, -1.0f), // -z
    };
    for (size_t i = 0; i < offsets.size(); ++i)
    {
        const glm::vec3& offset = offsets[i];
        const glm::vec3 neighbor = global_pos + offset;
        // Check (1) within chunk bounds, (2) already exists, and (3) is not visible.
        if (isInChunkBounds(neighbor) && isBlockPresent(neighbor) && !isBlockVisible(neighbor))
        {
            visibleBlocks.emplace(neighbor);
        }
        else if (!isInChunkBounds(neighbor) && neighboringChunks[i] != nullptr)
        {
            // TODO: instead of regenerating if needed, try to get it from persistent storage.
            // TODO: don't like how a chunk can modify its neighbors.
            if (neighboringChunks[i]->blockCount > 0 && neighboringChunks[i]->blocks == nullptr)
            {
                neighboringChunks[i]->init();
            }
            if (neighboringChunks[i]->isBlockPresent(neighbor) && !neighboringChunks[i]->isBlockVisible(neighbor))
            {
                neighboringChunks[i]->visibleBlocks.emplace(neighbor);
            }
        }
    }

    // Delete the container if there are no more blocks in this chunk.
    if (blockCount == 0)
    {
        blocks.reset();
    }
}

bool Chunk::isBlockOnEdge(const glm::vec3& global_pos, const Axis axis) const
{
    // 3 represents the 1st 3 positive axes in the enumeration.
    return (axis < 3) ? (global_pos[axis] == maxBounds[axis]) : (global_pos[axis - 3] == minBounds[axis - 3]);
}

bool Chunk::isBlockOnEdge(const glm::vec3& global_pos) const
{
    return isBlockOnEdge(global_pos, Axis::POS_X) || isBlockOnEdge(global_pos, Axis::NEG_X) ||
           isBlockOnEdge(global_pos, Axis::POS_Y) || isBlockOnEdge(global_pos, Axis::NEG_Y) ||
           isBlockOnEdge(global_pos, Axis::POS_Z) || isBlockOnEdge(global_pos, Axis::NEG_Z);
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

const Model Chunk::getModel() const
{
    std::vector<Model::Vertex> vertices;
    std::vector<Model::Index> indices;

    constexpr std::array<glm::vec3, 6> offsets = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // +x
        glm::vec3(0.0f, 1.0f, 0.0f),  // +y
        glm::vec3(0.0f, 0.0f, 1.0f),  // +z
        glm::vec3(-1.0f, 0.0f, 0.0f), // -x
        glm::vec3(0.0f, -1.0f, 0.0f), // -y
        glm::vec3(0.0f, 0.0f, -1.0f), // -z
    };
    constexpr std::array<glm::vec2, 4> uvs = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
    };

    // Iterate through the visible blocks to generate visible faces.
    for (const auto& block_pos : visibleBlocks)
    {
        const bool is_edge_block = isBlockOnEdge(block_pos);

        // Find the visible faces and generate vertices for the faces.
        for (unsigned i = 0; i < 6; ++i)
        {
            const glm::vec3& offset = offsets[i];
            const glm::vec3 neighbor = block_pos + offset;

            // Determine if this face should be visible and generate it.
            // If the neighbor is present in this chunk, then there is a neighbor,
            // otherwise, if the neighbor is not in this chunk bounds and (the neighboring chunk `i` is not null or the
            // neighbor is present in the neighboring chunk), then we consider that there is a neighbor.
            const bool has_neighbor = isBlockPresent(neighbor) ||
                                      (!isInChunkBounds(neighbor) && ((neighboringChunks[i] == nullptr) ||
                                                                      neighboringChunks[i]->isBlockPresent(neighbor)));
            if (!has_neighbor)
            {
                const glm::vec3 offset_to_face = offset / 2.0f; // Face is inbetween current and neighbor.
                std::array<glm::vec3, 4> v_offsets{};
                glm::vec3 normal;

                // Generating counter-clockwise.
                switch (i)
                {
                case 0: { // +x
                    v_offsets[0] = glm::vec3(0.0f, 0.5f, -0.5f);
                    v_offsets[1] = glm::vec3(0.0f, 0.5f, 0.5f);
                    v_offsets[2] = glm::vec3(0.0f, -0.5f, 0.5f);
                    v_offsets[3] = glm::vec3(0.0f, -0.5f, -0.5f);
                    normal = glm::vec3(1.0f, 0.0f, 0.0f);
                    break;
                }
                case 1: { // +y
                    v_offsets[0] = glm::vec3(-0.5f, 0.0f, -0.5f);
                    v_offsets[1] = glm::vec3(-0.5f, 0.0f, 0.5f);
                    v_offsets[2] = glm::vec3(0.5f, 0.0f, 0.5f);
                    v_offsets[3] = glm::vec3(0.5f, 0.0f, -0.5f);
                    normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    break;
                }
                case 2: { // +z
                    v_offsets[0] = glm::vec3(-0.5f, 0.5f, 0.0f);
                    v_offsets[1] = glm::vec3(-0.5f, -0.5f, 0.0f);
                    v_offsets[2] = glm::vec3(0.5f, -0.5f, 0.0f);
                    v_offsets[3] = glm::vec3(0.5f, 0.5f, 0.0f);
                    normal = glm::vec3(0.0f, 0.0f, 1.0f);
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
                case 4: { // -y
                    v_offsets[0] = glm::vec3(-0.5f, 0.0f, -0.5f);
                    v_offsets[1] = glm::vec3(0.5f, 0.0f, -0.5f);
                    v_offsets[2] = glm::vec3(0.5f, 0.0f, 0.5f);
                    v_offsets[3] = glm::vec3(-0.5f, 0.0f, 0.5f);
                    normal = glm::vec3(0.0f, -1.0f, 0.0f);
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

                assert(getBlock(block_pos).lock() != nullptr);
                const glm::vec3 color = getBlock(block_pos).lock()->color;
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

    return Model(vertices, indices);
}
