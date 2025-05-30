#include "box.hpp"

Box3d::Box3d(const glm::vec3& min_bounds, const glm::vec3& max_bounds)
    : Shape3d(Shape::Type::BOX_3D), minBounds(min_bounds), maxBounds(max_bounds)
{
}

glm::vec3 Box3d::getMinBounds() const
{
    return minBounds;
}

glm::vec3 Box3d::getMaxBounds() const
{
    return maxBounds;
}
