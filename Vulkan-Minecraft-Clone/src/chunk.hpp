#pragma once

#include "block.hpp"
#include "engine/physics/ray/ray.hpp"
#include "noise.hpp"

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
    glm::vec2 position;
    int size;
    glm::vec2 x_bounds;
    glm::vec2 z_bounds;

    // Contains blocks in this chunk and edge blocks of neighboring chunks.
    std::unique_ptr<std::unordered_map<glm::vec3, Block>> blockMap;

    // Exclusive to only blocks in this chunk.
    std::unordered_set<glm::vec3> visibleBlocks;

    bool checkBlockHidden(const glm::vec3& block_pos) const;
    bool checkInChunkBounds(const glm::vec3& block_pos) const;
    bool checkInEdgeBounds(const glm::vec3& block_pos) const; // Chunk bounds but includes neighboring edge blocks.

  public:
    Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size);

    void addBlock(const glm::vec3 block_pos);
    void removeBlock(const glm::vec3 block_pos);

    const std::optional<glm::vec3> getReachableBlock(const Ray& ray, glm::ivec3* face_entered = nullptr) const;

    glm::vec2 getPos() const;
    std::string getPosStr() const;

    const std::list<glm::vec3> getVisibleBlockPositions() const;
};
