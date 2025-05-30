#pragma once

#include "shape.hpp"

#include "../../../global.hpp" // TODO:

class Box3d : public Shape3d
{
  private:
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

  public:
    Box3d(const glm::vec3& min_bounds, const glm::vec3& max_bounds);

    glm::vec3 getMinBounds() const;
    glm::vec3 getMaxBounds() const;
};
