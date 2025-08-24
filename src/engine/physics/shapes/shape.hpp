#pragma once

#include "../geometry.hpp"

#include "../../usage/glm-usage.hpp"

#include <string>

class Shape : public Geometry
{
  private:
    Geometry::Type type = Geometry::Type::SHAPE;

  public:
    enum class Type
    {
        AABB_3D,
        PLANE_3D,
    };

    static std::string toString(const Shape::Type& type)
    {
        switch (type)
        {
        case Type::AABB_3D:
            return "AABB_3D";
        case Type::PLANE_3D:
            return "PLANE_3D";
        default:
            return "UNDEFINED";
        }
    }

    Geometry::Type getGeometryType() const;

    virtual Shape::Type getShapeType() const = 0;
};

class Shape3d : public Shape
{
  private:
    Shape::Type type;

  protected:
    Shape3d(const Shape::Type type);

  public:
    virtual void translate(const glm::vec3& value) = 0;
    Shape::Type getShapeType() const;
};
