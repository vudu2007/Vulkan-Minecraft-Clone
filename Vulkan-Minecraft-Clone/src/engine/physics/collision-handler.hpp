#pragma once

#include "./ray/ray.hpp"
#include "./shapes/box.hpp"

class CollisionHandler
{
  private:
    static bool rayBox3dIntersect(const Ray& ray, const Box3d& box);

  public:
    static bool rayShapeIntersect(const Ray& ray, const Shape& shape);
    static bool rayShapeIntersect(const Shape& shape, const Ray& ray);
    static bool shapeRayIntersect(const Shape& shape, const Ray& ray);
    static bool shapeRayIntersect(const Ray& ray, const Shape& shape);

    static bool shapeIntersect(const Shape& one, const Shape& two);

    CollisionHandler() = delete;
};
