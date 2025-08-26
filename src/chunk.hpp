#pragma once

#include "block.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/renderer/model.hpp"

#include "FastNoiseLite.h"
#include "engine/usage/glm-usage.hpp"
#include <glm/gtx/hash.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum Axis : uint8_t
{
    POS_X = 0,
    POS_Y = 1,
    POS_Z = 2,
    NEG_X = 3,
    NEG_Y = 4,
    NEG_Z = 5,
};

using ChunkCenter = glm::vec3;

class Chunk
{
  private:
    static constexpr glm::vec3 COLOR_DEFAULT{1.0f};
    static constexpr glm::vec3 COLOR_RED{1.0f, 0.0f, 0.0f};
    static constexpr glm::vec3 COLOR_GRASS{0.349f, 0.651f, 0.290f};
    static constexpr glm::vec3 COLOR_DIRT{0.396f, 0.263f, 0.129f};
    static constexpr glm::vec3 COLOR_STONE{0.439f, 0.502f, 0.565f};
    static constexpr glm::vec3 COLOR_SAND{0.96f, 0.87f, 0.70f};
    static constexpr int SEA_LEVEL = 0;
    static constexpr int HEIGHT_RANGE = 100;
    static constexpr int HEIGHT_OFFSET = -50;

    enum class BlockType : uint8_t
    {
        EMPTY,
        DEFAULT,
        RED,
        GRASS,
        DIRT,
        STONE,
        SAND,
    };
    static inline const std::unordered_map<BlockType, std::shared_ptr<Block>> BLOCK_PALETTE = {
        {BlockType::EMPTY,   nullptr                               },
        {BlockType::DEFAULT, std::make_shared<Block>(COLOR_DEFAULT)},
        {BlockType::RED,     std::make_shared<Block>(COLOR_RED)    },
        {BlockType::GRASS,   std::make_shared<Block>(COLOR_GRASS)  },
        {BlockType::DIRT,    std::make_shared<Block>(COLOR_DIRT)   },
        {BlockType::STONE,   std::make_shared<Block>(COLOR_STONE)  },
        {BlockType::SAND,    std::make_shared<Block>(COLOR_SAND)   },
    };

  public:
    std::array<Chunk*, 6> neighboringChunks{}; // Order: +x, +y, +z, -x, -y, -z.

  private:
    ChunkCenter center;
    int size;
    int blockCount = 0; // Includes edge blocks.

    // Bounds are inclusive and do not include edge blocks.
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    // Contains blocks in this chunk.
    using BlockContainer = std::vector<BlockType>;
    std::unique_ptr<BlockContainer> blocks;

    // Stores an index into `blocks`.
    std::unordered_set<glm::vec3> visibleBlocks;

    void initContainer();

    // --- TODO: TEMP methods and variables.
    const FastNoiseLite& heightNoise;
    void init();
    // --- TODO: END OF TEMP.

    int getBlockIndex(const glm::vec3& global_pos) const;
    BlockType getBlockType(const glm::vec3& global_pos) const;
    std::weak_ptr<Block> getBlock(const glm::vec3& global_pos) const;
    glm::ivec3 getLocalPos(const glm::vec3& global_pos) const;

    bool generateBlock(const glm::vec3& global_pos, const BlockType type);
    bool resetBlock(const glm::vec3& global_pos);

    bool isBlockPresent(const glm::vec3& global_pos) const;
    bool isBlockVisible(const glm::vec3& global_pos) const;
    bool isBlockHidden(const glm::vec3& global_pos) const;
    bool isBlockHidden(const glm::vec3& global_pos, const std::unordered_set<glm::vec3>& neighboring_blocks) const;
    bool isInChunkBounds(const glm::vec3& block_pos) const;

  public:
    Chunk(const FastNoiseLite& height_noise, const glm::vec3& center_pos, const int size);

    void addBlock(const glm::vec3& global_pos);
    void removeBlock(const glm::vec3& global_pos);

    bool isBlockOnEdge(const glm::vec3& global_pos, const Axis axis) const;
    bool isBlockOnEdge(const glm::vec3& global_pos) const;

    const std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr) const;
    bool doesEntityIntersect(
        const glm::vec3& velocity,
        const float delta,
        const Aabb3d& hitbox,
        float& new_delta,
        glm::vec3* normal = nullptr) const;

    ChunkCenter getCenter() const;
    const Model getModel() const;
};
