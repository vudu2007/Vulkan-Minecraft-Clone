#include "ray.hpp"

Ray::Ray(const glm::vec3 origin, const glm::vec3 direction, const float min, const float max)
    : origin(origin), direction(direction), min(min), max(max)
{
}

Geometry::Type Ray::getGeometryType() const
{
    return type;
}
