#include "shape.hpp"

// !----- Shape class.
Geometry::Type Shape::getGeometryType() const
{
    return type;
}

// !----- Shape3d class.
Shape3d::Shape3d(const Shape::Type type) : type(type)
{
}

Shape::Type Shape3d::getShapeType() const
{
    return type;
}
