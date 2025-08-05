#pragma once

#include "ray/ray.hpp"
#include "shapes/aabb.hpp"

class CollisionHandler
{
  private:
    static bool rayToBox3dIntersect(
        const Ray& ray,
        const Aabb3d& box,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr,
        glm::ivec3* out_face_enter = nullptr,
        glm::ivec3* out_face_exit = nullptr);

    static bool box3dToBox3dIntersect(const Aabb3d& box_1, const Aabb3d& box_2);

  public:
    static bool rayToShapeIntersect(
        const Ray& ray,
        const Shape& shape,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr,
        glm::ivec3* out_face_enter = nullptr, // TODO: these face parameters don't make sense for a general method.
        glm::ivec3* out_face_exit = nullptr);

    static bool shapeToShapeIntersect(const Shape& one, const Shape& two);

    CollisionHandler() = delete;
};
