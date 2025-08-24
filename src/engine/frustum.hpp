#pragma once

#include "physics/shapes/aabb.hpp"
#include "physics/shapes/plane.hpp"

class Frustum
{
  private:
    Plane3d nearPlane;
    Plane3d farPlane;
    Plane3d topPlane;
    Plane3d bottomPlane;
    Plane3d leftPlane;
    Plane3d rightPlane;

  public:
    Frustum(
        const glm::vec3& origin,
        const glm::vec3& forward,
        const glm::vec3& up,
        const glm::vec3& right,
        const float z_near,
        const float z_far,
        const float aspect,
        const float fov_y);

    bool isAabbInside(const Aabb3d& aabb) const;
    void translate(const glm::vec3& units);
};
