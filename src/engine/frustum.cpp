#include "frustum.hpp"

#include "physics/collision-handler.hpp"

#include <glm/gtx/string_cast.hpp>

#include <iostream>

Frustum::Frustum(
    const glm::vec3& origin,
    const glm::vec3& forward,
    const glm::vec3& up,
    const glm::vec3& right,
    const float z_near,
    const float z_far,
    const float aspect,
    const float fov_y)
{
    const float half_y = z_far * std::tan(fov_y * 0.5f);
    const float half_x = half_y * aspect;
    const glm::vec3 near_vec = z_near * forward;
    const glm::vec3 far_vec = z_far * forward;

    // Normals will point towards the frustum volume.
    nearPlane = Plane3d(forward, origin + near_vec);
    farPlane = Plane3d(-forward, origin + far_vec);
    topPlane = Plane3d(glm::cross(far_vec + up * half_y, right), origin);
    bottomPlane = Plane3d(glm::cross(right, far_vec - up * half_y), origin);
    leftPlane = Plane3d(glm::cross(far_vec - right * half_x, up), origin);
    rightPlane = Plane3d(glm::cross(up, far_vec + right * half_x), origin);
}

bool Frustum::isAabbInside(const Aabb3d& aabb) const
{
    return CollisionHandler::aabbToPlaneIntersect(aabb, nearPlane, true) &&
           CollisionHandler::aabbToPlaneIntersect(aabb, farPlane, true) &&
           CollisionHandler::aabbToPlaneIntersect(aabb, topPlane, true) &&
           CollisionHandler::aabbToPlaneIntersect(aabb, bottomPlane, true) &&
           CollisionHandler::aabbToPlaneIntersect(aabb, leftPlane, true) &&
           CollisionHandler::aabbToPlaneIntersect(aabb, rightPlane, true);
}

void Frustum::translate(const glm::vec3& units)
{
    nearPlane.translate(units);
    farPlane.translate(units);
    topPlane.translate(units);
    bottomPlane.translate(units);
    leftPlane.translate(units);
    rightPlane.translate(units);
}
