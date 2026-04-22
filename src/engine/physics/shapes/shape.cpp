#include "shape.hpp"

// !----- Shape class.
Shape::~Shape()
{}

std::string Shape::toString(const Shape::Type type)
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

Geometry::Type Shape::getGeometryType() const
{
    return type;
}

// !----- Shape3d class.
Shape3d::Shape3d(const Shape::Type type) : type(type)
{}

Shape3d::~Shape3d()
{}

Shape::Type Shape3d::getShapeType() const
{
    return type;
}
