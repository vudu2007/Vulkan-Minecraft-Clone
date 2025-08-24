#include "plane.hpp"

#include <limits>

Plane3d::Plane3d()
    : Shape3d(Shape::Type::PLANE_3D), normal(glm::vec3(std::numeric_limits<float>::quiet_NaN())),
      distance(std::numeric_limits<float>::quiet_NaN())
{
}

Plane3d::Plane3d(const glm::vec3& normal, const glm::vec3& point)
    : Shape3d(Shape::Type::PLANE_3D), normal(glm::normalize(normal)), distance(glm::dot(this->normal, point))
{
}

void Plane3d::translate(const glm::vec3& units)
{
    distance += glm::dot(normal, units);
}

glm::vec3 Plane3d::getNormal() const
{
    return normal;
}

float Plane3d::getDistance() const
{
    return distance;
}
