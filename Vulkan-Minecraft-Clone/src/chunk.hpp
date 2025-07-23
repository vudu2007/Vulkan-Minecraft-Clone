#pragma once

#include "block.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/renderer/model.hpp"

#include "FastNoiseLite.h"
#include <GLM/gtx/hash.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using ChunkCoord = glm::vec3; // Center position of chunk floored as a string.
using ChunkCenter = glm::vec3;

class Chunk
{
  private:
    // Edge blocks refer to blocks in neighboring chunks.
    static constexpr int NUM_EDGE_BLOCKS = 2;

    static constexpr glm::vec3 COLOR_GRASS{0.349f, 0.651f, 0.290f};
    static constexpr glm::vec3 COLOR_DIRT{0.396f, 0.263f, 0.129f};
    static constexpr glm::vec3 COLOR_STONE{0.439f, 0.502f, 0.565f};
    static constexpr glm::vec3 COLOR_SAND{0.96f, 0.87f, 0.70f};
    static constexpr int SEA_LEVEL = 0;
    static constexpr int HEIGHT_RANGE = 100;
    static constexpr int HEIGHT_OFFSET = -50;

    ChunkCenter center;
    int size;

    // Bounds are inclusive and do not include edge blocks.
    glm::vec2 xBounds;
    glm::vec2 yBounds;
    glm::vec2 zBounds;

    // Mesh info.
    std::vector<Model::Vertex> vertices;
    std::vector<Model::Index> indices;

    // Contains blocks in this chunk and edge blocks of neighboring chunks.
    std::unique_ptr<std::unordered_map<glm::vec3, Block>> blockMap;

    // Exclusive to only blocks in this chunk.
    std::unordered_set<glm::vec3> visibleBlocks;

    bool checkBlockHidden(const glm::vec3& block_pos) const;
    bool checkInChunkBounds(const glm::vec3& block_pos) const;
    bool checkInEdgeBounds(const glm::vec3& block_pos) const; // Chunk bounds but includes neighboring edge blocks.

    void generateMesh();

  public:
    Chunk(const FastNoiseLite& height_noise, const glm::vec3& center_pos, const int size);

    void addBlock(const glm::vec3 block_pos);
    void removeBlock(const glm::vec3 block_pos);

    const std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr) const;

    const std::vector<Model::Vertex> getVertices() const;
    const std::vector<Model::Index> getIndices() const;
};
