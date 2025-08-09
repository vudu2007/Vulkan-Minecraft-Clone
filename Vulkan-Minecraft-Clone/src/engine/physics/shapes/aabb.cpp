#include "aabb.hpp"

Aabb3d::Aabb3d(const glm::vec3& min_bounds, const glm::vec3& max_bounds)
    : Shape3d(Shape::Type::AABB_3D), minBounds(min_bounds), maxBounds(max_bounds)
{
    assert(
        min_bounds.x < max_bounds.x && min_bounds.y < max_bounds.y && min_bounds.z < max_bounds.z &&
        "Aabb3d constructor warning: 1+ components of min bounds is greater than component(s) of max bounds!");
}

void Aabb3d::translate(const glm::vec3& units)
{
    minBounds += units;
    maxBounds += units;
}

glm::vec3 Aabb3d::getMinBounds() const
{
    return minBounds;
}

glm::vec3 Aabb3d::getMaxBounds() const
{
    return maxBounds;
}

glm::vec3 Aabb3d::getCenter() const
{
    return (maxBounds + minBounds) * 0.5f;
}

glm::vec3 Aabb3d::getLengths() const
{
    return (maxBounds - minBounds) * 0.5f;
}
