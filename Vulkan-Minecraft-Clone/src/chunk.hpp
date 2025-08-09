#pragma once

#include "block.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/renderer/model.hpp"

#include "FastNoiseLite.h"
#include "engine/usage/glm-usage.hpp"
#include <GLM/gtx/hash.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using ChunkCenter = glm::vec3;

class Chunk
{
  private:
    using BlockContainer = std::vector<std::vector<std::vector<std::shared_ptr<Block>>>>;

    // Edge blocks refer to blocks in neighboring chunks.
    static constexpr int EDGE_OFFSET = 2;

    static constexpr glm::vec3 COLOR_DEFAULT{1.0f};
    static constexpr glm::vec3 COLOR_RED{1.0f, 0.0f, 0.0f};
    static constexpr glm::vec3 COLOR_GRASS{0.349f, 0.651f, 0.290f};
    static constexpr glm::vec3 COLOR_DIRT{0.396f, 0.263f, 0.129f};
    static constexpr glm::vec3 COLOR_STONE{0.439f, 0.502f, 0.565f};
    static constexpr glm::vec3 COLOR_SAND{0.96f, 0.87f, 0.70f};
    static constexpr int SEA_LEVEL = 0;
    static constexpr int HEIGHT_RANGE = 100;
    static constexpr int HEIGHT_OFFSET = -50;

    enum BlockType
    {
        DEFAULT,
        RED,
        GRASS,
        DIRT,
        STONE,
        SAND,
    };
    static inline const std::unordered_map<BlockType, std::shared_ptr<Block>> AVAILABLE_BLOCKS = {
        {BlockType::DEFAULT, std::make_shared<Block>(COLOR_DEFAULT)},
        {BlockType::RED,     std::make_shared<Block>(COLOR_RED)    },
        {BlockType::GRASS,   std::make_shared<Block>(COLOR_GRASS)  },
        {BlockType::DIRT,    std::make_shared<Block>(COLOR_DIRT)   },
        {BlockType::STONE,   std::make_shared<Block>(COLOR_STONE)  },
        {BlockType::SAND,    std::make_shared<Block>(COLOR_SAND)   },
    };

    ChunkCenter center;
    int size;
    int blockCount = 0; // Includes edge blocks.

    // Bounds are inclusive and do not include edge blocks.
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    // Mesh info.
    Model model;

    // Contains blocks in this chunk and edge blocks of neighboring chunks.
    // Will keep edge blocks up-to-date with neighbors based on player's interaction with the world.
    // Stored in the order x -> y -> z and maintain a size of (`size` + `EDGE_OFFSET` * 2) cubed.
    std::unique_ptr<BlockContainer> blocks;

    // Exclusive to only blocks in this chunk.
    std::unordered_set<glm::vec3> visibleBlocks;

    void initContainer();

    std::shared_ptr<Block> getBlock(const glm::vec3& global_pos) const;
    glm::vec3 getLocalPos(const glm::vec3& global_pos) const;

    void generateBlock(const glm::vec3& global_pos, const BlockType type);
    void resetBlock(const glm::vec3& global_pos);

    bool doesBlockExist(const glm::vec3& global_pos) const;
    bool isBlockVisible(const glm::vec3& global_pos) const; // In the visible blocks set.
    bool isBlockHidden(const glm::vec3& global_pos) const;  // Hidden relative to other blocks.
    bool isInChunkBounds(const glm::vec3& block_pos) const;
    bool isInEdgeBounds(const glm::vec3& block_pos) const; // Chunk bounds but includes neighboring edge blocks.

    void generateMesh();

  public:
    Chunk(const FastNoiseLite& height_noise, const glm::vec3& center_pos, const int size);

    void addBlock(const glm::vec3& global_pos);
    void removeBlock(const glm::vec3& global_pos);

    const std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr) const;
    bool doesEntityIntersect(
        const glm::vec3& velocity,
        const float delta,
        const Aabb3d& hitbox,
        float& new_delta,
        glm::ivec3* entry_face = nullptr) const;

    ChunkCenter getCenter() const;
    const Model& getModel() const;
};
