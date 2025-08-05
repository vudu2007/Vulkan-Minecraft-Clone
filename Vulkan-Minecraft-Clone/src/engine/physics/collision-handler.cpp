#include "collision-handler.hpp"

#include <limits>
#include <stdexcept>

bool CollisionHandler::rayToBox3dIntersect(
    const Ray& ray,
    const Aabb3d& box,
    float* out_t_min,
    float* out_t_max,
    glm::ivec3* out_face_enter,
    glm::ivec3* out_face_exit)
{
    if (out_face_enter != nullptr)
    {
        *out_face_enter = glm::ivec3(0);
    }

    const glm::vec3 ray_o = ray.getOrigin();
    const glm::vec3 ray_d = ray.getDirection();
    const glm::vec3 min_bounds = box.getMinBounds();
    const glm::vec3 max_bounds = box.getMaxBounds();

    float t_min = ray.getMin();
    float t_max = ray.getMax();

    // Check the x, y, and z planes.
    for (unsigned i = 0; i < 3; ++i)
    {
        const float inv_ray_d_component = 1.0f / ray_d[i]; // Handles -0 vs 0.

        // Depending on the ray's direction, the `t_min` could be `>` than `t_max`.
        float next_t_min = (min_bounds[i] - ray_o[i]) * inv_ray_d_component;
        float next_t_max = (max_bounds[i] - ray_o[i]) * inv_ray_d_component;
        if (inv_ray_d_component < 0)
        {
            std::swap(next_t_min, next_t_max);
        }

        t_max = std::min(t_max, next_t_max);
        if (next_t_min > t_min)
        {
            if (out_face_enter != nullptr)
            {
                *out_face_enter = glm::ivec3(0);
                (*out_face_enter)[i] = (inv_ray_d_component < 0) ? 1 : -1;
            }
            t_min = next_t_min;
        }

        if (t_min > t_max)
        {
            // The ray doesn't intersect the box; intersects the portion of the planes not a part of the box.
            if (out_t_min != nullptr)
            {
                *out_t_min = std::numeric_limits<float>::infinity();
            }
            if (out_t_max != nullptr)
            {
                *out_t_max = std::numeric_limits<float>::infinity();
            }
            return false;
        }
    }
    if (out_t_min != nullptr)
    {
        *out_t_min = t_min;
    }
    if (out_t_max != nullptr)
    {
        *out_t_max = t_max;
    }
    return true;
}

bool CollisionHandler::box3dToBox3dIntersect(const Aabb3d& box_1, const Aabb3d& box_2)
{
    const auto box_1_min = box_1.getMinBounds();
    const auto box_1_max = box_1.getMaxBounds();
    const auto box_2_min = box_2.getMinBounds();
    const auto box_2_max = box_2.getMaxBounds();
    const bool does_x_overlap = (box_1_min.x <= box_2_max.x) && (box_2_min.x <= box_1_max.x);
    const bool does_y_overlap = (box_1_min.y <= box_2_max.y) && (box_2_min.y <= box_1_max.y);
    const bool does_z_overlap = (box_1_min.z <= box_2_max.z) && (box_2_min.z <= box_1_max.z);
    return does_x_overlap && does_y_overlap && does_z_overlap;
}

bool CollisionHandler::rayToShapeIntersect(
    const Ray& ray,
    const Shape& shape,
    float* out_t_min,
    float* out_t_max,
    glm::ivec3* out_face_enter,
    glm::ivec3* out_face_exit)
{
    switch (shape.getShapeType())
    {
    case Shape::Type::AABB_3D:
        return rayToBox3dIntersect(
            ray,
            static_cast<const Aabb3d&>(shape),
            out_t_min,
            out_t_max,
            out_face_enter,
            out_face_exit);
    default:
        throw std::runtime_error(
            "Unsupported shape for a shape-ray intersection test: " + Shape::toString(shape.getShapeType()));
    }
    return false;
}

bool CollisionHandler::shapeToShapeIntersect(const Shape& one, const Shape& two)
{
    // TODO: find a more scalable way to implement this.
    if ((one.getShapeType() == Shape::Type::AABB_3D) && (two.getShapeType() == Shape::Type::AABB_3D))
    {
        // Box3D and Box3D.
        return box3dToBox3dIntersect(static_cast<const Aabb3d&>(one), static_cast<const Aabb3d&>(two));
    }
    else
    {
        // Intersection not supported/unimplemented.
        throw std::runtime_error(
            "Unsupported shape combination for shape-to-shape intersection test: (one) " +
            Shape::toString(one.getShapeType()) + " and (two) " + Shape::toString(two.getShapeType()));
    }
    return false;
}
