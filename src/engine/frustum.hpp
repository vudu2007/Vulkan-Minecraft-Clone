#pragma once

#include "physics/shapes/aabb.hpp"
#include "physics/shapes/plane.hpp"

class Frustum
{
  public:
    Frustum(
        const glm::vec3& origin,
        const glm::vec3& forward,
        const glm::vec3& up,
        const glm::vec3& right,
        float z_near,
        float z_far,
        float aspect,
        float fov_y);

    bool isAabbInside(const Aabb3d& aabb) const;
    void translate(const glm::vec3& units);

  private:
    Plane3d nearPlane;
    Plane3d farPlane;
    Plane3d topPlane;
    Plane3d bottomPlane;
    Plane3d leftPlane;
    Plane3d rightPlane;
};
