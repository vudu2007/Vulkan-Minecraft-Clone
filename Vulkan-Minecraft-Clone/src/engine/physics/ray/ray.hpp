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

    void setOrigin(const glm::vec3& o);    // TODO: implement a better solution.
    void setDirection(const glm::vec3& r); // TODO: implement a better solution.

    glm::vec3 getOrigin() const;
    glm::vec3 getDirection() const;
    float getMin() const;
    float getMax() const;
};
