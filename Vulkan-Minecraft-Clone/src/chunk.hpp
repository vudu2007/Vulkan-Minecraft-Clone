#pragma once

#include "block.hpp"
#include "engine/physics/ray/ray.hpp"
#include "engine/renderer/model.hpp"

#include "FastNoiseLite.h"
#include <GLM/gtx/hash.hpp>

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Chunk
{
  private:
    static constexpr int NUM_EDGE_BLOCKS = 2;

    glm::vec3 position;
    int size;
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

    glm::vec3 getPos() const;
    std::string getPosStr() const;

    const std::list<glm::vec3> getVisibleBlockPositions() const;
    const std::vector<Model::Vertex> getVertices() const;
    const std::vector<Model::Index> getIndices() const;
};
