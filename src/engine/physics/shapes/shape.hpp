#pragma once

#include "../geometry.hpp"

#include "../../usage/glm-usage.hpp"

#include <string>

class Shape : public Geometry
{
  public:
    enum class Type
    {
        AABB_3D,
        PLANE_3D,
    };

    virtual ~Shape();

    static std::string toString(Shape::Type type);

    Geometry::Type getGeometryType() const;

    virtual Shape::Type getShapeType() const = 0;

  private:
    Geometry::Type type = Geometry::Type::SHAPE;
};

class Shape3d : public Shape
{
  public:
    virtual ~Shape3d();

    virtual void translate(const glm::vec3& value) = 0;
    Shape::Type getShapeType() const;

  protected:
    Shape3d(Shape::Type type);

  private:
    Shape::Type type;
};
