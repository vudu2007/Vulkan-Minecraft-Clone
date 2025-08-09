#pragma once

#include "ray/ray.hpp"
#include "shapes/aabb.hpp"

#include <any>
#include <vector>

class CollisionHandler
{
  private:
    static bool rayToAabb3dIntersect(
        const Ray& ray,
        const Aabb3d& aabb,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr,
        glm::ivec3* out_face_enter = nullptr,
        glm::ivec3* out_face_exit = nullptr);

    static bool aabb3dToAabb3dIntersect(
        const Aabb3d& a,
        const Aabb3d& b,
        const bool consider_edges = true,
        std::vector<std::any>* a_info = nullptr,
        std::vector<std::any>* b_info = nullptr);

  public:
    static bool rayToShapeIntersect(
        const Ray& ray,
        const Shape& shape,
        float* out_t_min = nullptr,
        float* out_t_max = nullptr,
        glm::ivec3* out_face_enter = nullptr, // TODO: these face parameters don't make sense for a general method.
        glm::ivec3* out_face_exit = nullptr);

    static bool shapeToShapeIntersect(
        const Shape& a,
        const Shape& b,
        const bool consider_edges = true,
        std::vector<std::any>* a_info = nullptr,
        std::vector<std::any>* b_info = nullptr);

    static float sweptAABB(
        const Aabb3d& a,
        const Aabb3d& b,
        const glm::vec3& a_velocity,
        const float t,
        glm::ivec3* entry_face = nullptr);
    static float sweptAABB(
        const Aabb3d& a,
        const Aabb3d& b,
        const glm::vec3& a_velocity,
        glm::vec3& b_velocity,
        const float t,
        glm::ivec3* entry_face = nullptr);

    CollisionHandler() = delete;
};
