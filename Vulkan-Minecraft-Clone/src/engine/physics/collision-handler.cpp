#include "collision-handler.hpp"

#include <limits>
#include <stdexcept>

bool CollisionHandler::rayBox3dIntersect(
    const Ray& ray,
    const Box3d& box,
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
        float next_t_min;
        float next_t_max;
        if (inv_ray_d_component >= 0)
        {
            next_t_min = (min_bounds[i] - ray_o[i]) * inv_ray_d_component;
            next_t_max = (max_bounds[i] - ray_o[i]) * inv_ray_d_component;
        }
        else
        {
            next_t_min = (max_bounds[i] - ray_o[i]) * inv_ray_d_component;
            next_t_max = (min_bounds[i] - ray_o[i]) * inv_ray_d_component;
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

bool CollisionHandler::rayShapeIntersect(
    const Ray& ray,
    const Shape& shape,
    float* out_t_min,
    float* out_t_max,
    glm::ivec3* out_face_enter,
    glm::ivec3* out_face_exit)
{
    switch (shape.getShapeType())
    {
    case Shape::Type::BOX_3D:
        return rayBox3dIntersect(
            ray,
            static_cast<const Box3d&>(shape),
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

bool CollisionHandler::shapeIntersect(const Shape& one, const Shape& two)
{
    // TODO: implement
    throw std::runtime_error("shapeIntersect(...) not implemented!");
}
