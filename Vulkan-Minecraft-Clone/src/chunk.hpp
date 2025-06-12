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

    std::unique_ptr<std::unordered_map<glm::vec3, Block>> blockMap;
    std::unordered_set<glm::vec3> visibleBlocks;
    std::list<glm::vec3> hiddenBlocks;

  public:
    Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size);

    void removeBlock(const glm::vec3 block_pos);

    const std::optional<glm::vec3> getReachableBlock(const Ray& ray) const;

    glm::vec2 getPos() const;
    std::string getPosStr() const;

    const std::list<glm::vec3> getVisibleBlockPositions() const;
};
