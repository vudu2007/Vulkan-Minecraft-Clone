#pragma once

#include "../geometry.hpp"

#include <string>

class Shape : public Geometry
{
  private:
    Geometry::Type type = Geometry::Type::SHAPE;

  public:
    enum Type
    {
        BOX_3D,
    };

    static std::string toString(const Shape::Type& type)
    {
        switch (type)
        {
        case Type::BOX_3D:
            return "BOX_3D";
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
    Shape::Type getShapeType() const;
};
