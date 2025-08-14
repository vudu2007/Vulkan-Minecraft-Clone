#include "collision-handler.hpp"

#include <array>
#include <limits>
#include <stdexcept>

bool CollisionHandler::rayToAabb3dIntersect(
    const Ray& ray,
    const Aabb3d& aabb,
    float* out_t_min,
    float* out_t_max,
    glm::ivec3* out_face_enter,
    glm::ivec3* out_face_exit)
{
    const glm::vec3 ray_o = ray.getOrigin();
    const glm::vec3 ray_d = ray.getDirection();
    const glm::vec3 min_bounds = aabb.getMinBounds();
    const glm::vec3 max_bounds = aabb.getMaxBounds();

    float t_min = ray.getMin();
    float t_max = ray.getMax();

    // Check the x, y, and z planes.
    for (unsigned i = 0; i < 3; ++i)
    {
        const float inv_ray_d_component = 1.0f / ray_d[i]; // Handles -0 vs 0.

        // Depending on the ray's direction, the `t_min` could be `>` than `t_max`.
        float next_t_min = (min_bounds[i] - ray_o[i]) * inv_ray_d_component;
        float next_t_max = (max_bounds[i] - ray_o[i]) * inv_ray_d_component;
        if (inv_ray_d_component < 0.0f)
        {
            std::swap(next_t_min, next_t_max);
        }

        if (std::abs(next_t_min - next_t_max) < std::numeric_limits<float>::epsilon())
        {
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

        if (next_t_max < t_max)
        {
            if (out_face_exit != nullptr)
            {
                *out_face_exit = glm::ivec3(0);
                (*out_face_enter)[i] = (inv_ray_d_component < 0) ? 1 : -1;
            }
            t_max = next_t_max;
        }
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
            // The ray doesn't intersect the aabb; intersects the portion of the planes not a part of the aabb.
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

bool CollisionHandler::aabb3dToAabb3dIntersect(
    const Aabb3d& a,
    const Aabb3d& b,
    const bool consider_edges,
    std::vector<std::any>* a_info,
    std::vector<std::any>* b_info)
{
    constexpr glm::length_t dim = 3;
    const auto a_min = a.getMinBounds();
    const auto a_max = a.getMaxBounds();
    const auto b_min = b.getMinBounds();
    const auto b_max = b.getMaxBounds();

    // Run an overlap test.
    // Iterate through each axis (x -> y -> z).
    std::array<bool, dim> do_axes_overlap{};
    glm::vec3 overlap_vector{}; // Stores information relative to `a`.
    for (glm::length_t i = 0; i < dim; ++i)
    {
        const float k = a_min[i] - b_max[i];
        const float l = b_min[i] - a_max[i];
        const float k_abs = std::abs(k);
        const float l_abs = std::abs(l);
        overlap_vector[i] = (l_abs <= k_abs) ? l_abs : -k_abs;

        // Same condition as (a_min[i] <= b_max[i]) && (b_min[i] <= a_max[i]).
        do_axes_overlap[i] = consider_edges ? ((k <= 0.0f) && (l <= 0.0f)) : ((k < 0.0f) && (l < 0.0f));
    }

    const bool does_overlap = do_axes_overlap[0] && do_axes_overlap[1] && do_axes_overlap[2];

    // Put the results into `a_info` and `b_info`.
    if (does_overlap && (a_info != nullptr))
    {
        a_info->resize(dim);
        if (b_info != nullptr)
        {
            b_info->resize(dim);
        }

        for (glm::length_t i = 0; i < dim; ++i)
        {
            if (!do_axes_overlap[i])
            {
                continue;
            }

            (*a_info)[i] = overlap_vector[i];
            if (b_info != nullptr)
            {
                (*b_info)[i] = -overlap_vector[i];
            }
        }
    }

    // Return whether the AABBs overlapped.
    return does_overlap;
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
        return rayToAabb3dIntersect(
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

bool CollisionHandler::shapeToShapeIntersect(
    const Shape& a,
    const Shape& b,
    const bool consider_edges,
    std::vector<std::any>* a_info,
    std::vector<std::any>* b_info)
{
    if (a_info != nullptr)
    {
        a_info->clear();
    }
    if (b_info != nullptr)
    {
        b_info->clear();
    }

    // TODO: find a more scalable way to implement this.
    if ((a.getShapeType() == Shape::Type::AABB_3D) && (b.getShapeType() == Shape::Type::AABB_3D))
    {
        // Box3D and Box3D.
        return aabb3dToAabb3dIntersect(
            static_cast<const Aabb3d&>(a),
            static_cast<const Aabb3d&>(b),
            consider_edges,
            a_info,
            b_info);
    }
    else
    {
        // Intersection not supported/unimplemented.
        throw std::runtime_error(
            "Unsupported shape combination for shape-to-shape intersection test: (one) " +
            Shape::toString(a.getShapeType()) + " and (two) " + Shape::toString(b.getShapeType()));
    }
    return false;
}

[[nodiscard]] Aabb3d CollisionHandler::getBroadPhaseAabb(const Aabb3d& a, const glm::vec3& a_velocity, const float dt)
{
    const glm::vec3 displacement = a_velocity * dt;
    glm::vec3 min_bounds = a.getMinBounds();
    glm::vec3 max_bounds = a.getMaxBounds();

    for (glm::length_t axis = 0; axis < 3; ++axis)
    {
        // Add negative values to the min bounds and positive values to the max bounds.
        // Zero values are unaffecting.
        if (displacement[axis] < 0.0f)
        {
            min_bounds[axis] += displacement[axis];
        }
        else
        {
            max_bounds[axis] += displacement[axis];
        }
    }

    return Aabb3d(min_bounds, max_bounds);
}

float CollisionHandler::sweptAabbPerAxis(
    const Aabb3d& a,
    const Aabb3d& b,
    const glm::vec3& a_velocity,
    const float dt,
    glm::vec3& normal)
{
    normal = {};

    const glm::vec3 a_min_bounds = a.getMinBounds();
    const glm::vec3 b_min_bounds = b.getMinBounds();
    const glm::vec3 a_dim = a.getDim();
    const glm::vec3 b_dim = b.getDim();

    glm::vec3 inverse_entry{}; // Displacement between the closest edges of `a` and `b`.
    glm::vec3 inverse_exit{};  // Displacement between the furthest edges of `a` and `b`.
    glm::vec3 entry_times{};   // Contains potential `t_min` for each axis.
    glm::vec3 exit_times{};    // Contains potential `t_max` for each axis.

    for (glm::length_t axis = 0; axis < 3; ++axis)
    {
        // Calculate displacements needed for a collision.
        inverse_entry[axis] = b_min_bounds[axis] - (a_min_bounds[axis] + a_dim[axis]);
        inverse_exit[axis] = (b_min_bounds[axis] + b_dim[axis]) - a_min_bounds[axis];
        if (a_velocity[axis] <= 0.0f) // Reverse if velocity is going the other way.
        {
            std::swap(inverse_entry[axis], inverse_exit[axis]);
        }

        entry_times[axis] = -std::numeric_limits<float>::infinity();
        exit_times[axis] = std::numeric_limits<float>::infinity();
        if (a_velocity[axis] != 0.0f) // Make sure we don't divide by 0.
        {
            // Calculate time = displacement / velocity; amount of time before collision.
            entry_times[axis] = inverse_entry[axis] / a_velocity[axis];
            exit_times[axis] = inverse_exit[axis] / a_velocity[axis];
        }
    }

    // Get the the earliest and latest times; `t_min` and `t_max` respectively.
    const float t_min = std::max(std::max(entry_times.x, entry_times.y), entry_times.z);
    const float t_max = std::min(std::min(exit_times.x, exit_times.y), exit_times.z);

    // Check if there was no collision.
    if ((t_min > t_max) || (entry_times.x < 0.0f && entry_times.y < 0.0f && entry_times.z < 0.0f) ||
        (entry_times.x > dt || entry_times.y > dt || entry_times.z > dt))
    {
        return std::numeric_limits<float>::infinity();
    }

    // Otherwise, there was a collision.
    // Determine the normal.
    for (glm::length_t axis = 0; axis < 3; ++axis)
    {
        // Get the axis containing `t_min`.
        if (t_min == entry_times[axis])
        {
            // Determine if we can use `inverse_entry` to determine the normal.
            // If it's 0, there is overlap and we can't determine the normal, so use the velocity.
            if (inverse_entry[axis] == 0.0f)
            {
                assert(a_velocity[axis] != 0.0f);
                normal[axis] = (a_velocity[axis] < 0.0f) ? 1.0f : -1.0f;
            }
            else
            {
                normal[axis] = (inverse_entry[axis] < 0.0f) ? 1.0f : -1.0f;
            }

            break;
        }
    }

    return t_min;
}

float CollisionHandler::sweptAabbPerAxis(
    const Aabb3d& a,
    const Aabb3d& b,
    const glm::vec3& a_velocity,
    const glm::vec3& b_velocity,
    const float dt,
    glm::vec3& normal)
{
    // Calculate the relative velocity from the perspective of `b` and run the Swept AABB algorithm.
    // Given that `a` and `b` are dynamic, we convert `b` to be static.
    return sweptAabbPerAxis(a, b, (a_velocity - b_velocity), dt, normal);
}

float CollisionHandler::sweptAabbMinkowski(
    const Aabb3d& a,
    const Aabb3d& b,
    const glm::vec3& a_velocity,
    const float t,
    glm::ivec3* entry_face)
{
    // `a` should be dynamic and `b` should be static.

    // Expand the static AABB `b` using the Minkowski difference.
    const glm::vec3 expand_lens(a.getLength());
    const Aabb3d minkowski_aabb(b.getMinBounds() - expand_lens, b.getMaxBounds() + expand_lens);

    // Reduce the dynamic AABB `a` to a point, then into a ray using the it's velocity.
    const glm::vec3 origin = a.getCenter();
    const glm::vec3 direction = glm::normalize(a_velocity);
    const float t_max = glm::length(a_velocity * t);
    const Ray ray(origin, direction, 0.0f, t_max);

    // Run the ray to AABB intersection test.
    float t_min = std::numeric_limits<float>::infinity();
    rayToAabb3dIntersect(ray, minkowski_aabb, &t_min, nullptr, entry_face);

    // `t_min` will be a fraction of `t_max`.
    assert(t_max > 0.0f);
    const float fraction = t_min / t_max;

    // We use that fraction on the delta `t` so that the `t_min` is scaled to `t`.
    const float new_delta = fraction * t;

    return new_delta;
}

float CollisionHandler::sweptAabbMinkowski(
    const Aabb3d& a,
    const Aabb3d& b,
    const glm::vec3& a_velocity,
    const glm::vec3& b_velocity,
    const float t,
    glm::ivec3* entry_face)
{
    // Calculate the relative velocity from the perspective of `b` and run the Swept AABB algorithm.
    // Given that `a` and `b` are dynamic, we convert `b` to be static.
    return sweptAabbMinkowski(a, b, (a_velocity - b_velocity), t, entry_face);
}
