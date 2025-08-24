#pragma once

#include "shape.hpp"

class Plane3d : public Shape3d
{
  private:
    glm::vec3 normal;
    float distance; // Distance from the origin to the plane's nearest point.

  public:
    Plane3d();
    Plane3d(const glm::vec3& normal, const glm::vec3& point);

    void translate(const glm::vec3& units) override;

    glm::vec3 getNormal() const;
    float getDistance() const;
};
