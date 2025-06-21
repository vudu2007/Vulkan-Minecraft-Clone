#pragma once

#include "ray/ray.hpp"
#include "shapes/box.hpp"

class CollisionHandler
{
  private:
    static bool rayBox3dIntersect(
        const Ray& ray,
        const Box3d& box,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr,
        glm::ivec3* out_face_enter = nullptr,
        glm::ivec3* out_face_exit = nullptr);

  public:
    static bool rayShapeIntersect(
        const Ray& ray,
        const Shape& shape,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr,
        glm::ivec3* out_face_enter = nullptr, // TODO: these face parameters don't make sense for a general method.
        glm::ivec3* out_face_exit = nullptr);

    static bool shapeIntersect(const Shape& one, const Shape& two);

    CollisionHandler() = delete;
};
