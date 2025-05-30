#pragma once

#include "../geometry.hpp"

#include "../../../global.hpp" // TODO:

class Ray : public Geometry
{
  private:
    Geometry::Type type = Geometry::Type::RAY;

    glm::vec3 origin;
    glm::vec3 direction;
    float min;
    float max;

  public:
    Ray(const glm::vec3 origin, const glm::vec3 direction, const float min, const float max);

    Geometry::Type getGeometryType() const;
};
