#pragma once

#include "shape.hpp"

class Aabb3d : public Shape3d
{
  private:
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

  public:
    Aabb3d(const glm::vec3& min_bounds, const glm::vec3& max_bounds);

    void translate(const glm::vec3& units) override;

    glm::vec3 getMinBounds() const;
    glm::vec3 getMaxBounds() const;
    glm::vec3 getCenter() const;
    glm::vec3 getLength() const;
    glm::vec3 getDim() const;
};
