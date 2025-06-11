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
        float* out_t_max = nullptr);

  public:
    static bool rayShapeIntersect(
        const Ray& ray,
        const Shape& shape,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr);
    static bool rayShapeIntersect(
        const Shape& shape,
        const Ray& ray,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr);
    static bool shapeRayIntersect(
        const Shape& shape,
        const Ray& ray,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr);
    static bool shapeRayIntersect(
        const Ray& ray,
        const Shape& shape,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr);

    static bool shapeIntersect(const Shape& one, const Shape& two);

    CollisionHandler() = delete;
};
