#pragma once

#include "block.hpp"
#include "noise.hpp"

#include <GLM/gtx/hash.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

class Chunk
{
  private:
    glm::vec2 position;
    int size;

    std::unique_ptr<std::unordered_map<glm::vec3, Block>> blockMap;
    std::vector<glm::vec3> heightMap;
    std::list<Block*> visibleBlocks;
    std::list<Block*> hiddenBlocks;

  public:
    Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size);

    void merge(const std::vector<Chunk*>& others);

    glm::vec2 getPos() const;
    std::string getPosStr() const;

    const std::vector<glm::vec3>& getHeightMap() const;
    const std::list<Block*>& getVisibleBlocks() const;
};
