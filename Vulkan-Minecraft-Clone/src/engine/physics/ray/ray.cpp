#include "ray.hpp"

Ray::Ray(const glm::vec3 origin, const glm::vec3 direction, const float min, const float max)
    : origin(origin), direction(glm::normalize(direction)), min(min), max(max)
{
}

Geometry::Type Ray::getGeometryType() const
{
    return type;
}

void Ray::setOrigin(const glm::vec3& o)
{
    origin = o;
}

void Ray::setDirection(const glm::vec3& r)
{
    direction = glm::normalize(r);
}

glm::vec3 Ray::getOrigin() const
{
    return origin;
}

glm::vec3 Ray::getDirection() const
{
    return direction;
}

float Ray::getMin() const
{
    return min;
}

float Ray::getMax() const
{
    return max;
}
