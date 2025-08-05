#include "aabb.hpp"

Aabb3d::Aabb3d(const glm::vec3& min_bounds, const glm::vec3& max_bounds)
    : Shape3d(Shape::Type::AABB_3D), minBounds(min_bounds), maxBounds(max_bounds)
{
}

void Aabb3d::translate(const glm::vec3& value)
{
    // TODO
}

glm::vec3 Aabb3d::getMinBounds() const
{
    return minBounds;
}

glm::vec3 Aabb3d::getMaxBounds() const
{
    return maxBounds;
}
