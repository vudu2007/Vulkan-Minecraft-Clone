#pragma once

#include "noise.hpp"

#include <string>
#include <vector>

class Chunk
{
  private:
    glm::vec2 position;
    int size;

    std::vector<glm::vec3> heightMap;

  public:
    Chunk() = default;
    Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size);

    glm::vec2 getPos() const;
    std::string getPosStr() const;

    const std::vector<glm::vec3>& getHeightMap() const;
};
