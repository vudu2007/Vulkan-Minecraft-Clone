#include "collision-handler.hpp"

#include <iostream>
#include <stdexcept>

bool CollisionHandler::rayBox3dIntersect(const Ray& ray, const Box3d& box)
{
    // TODO: implement
    return false;
}

bool CollisionHandler::rayShapeIntersect(const Ray& ray, const Shape& shape)
{
    switch (shape.getShapeType())
    {
    case Shape::Type::BOX_3D:
        return rayBox3dIntersect(ray, static_cast<const Box3d&>(shape));
    default:
        throw std::runtime_error(
            "Unsupported shape for a shape-ray intersection test: " + Shape::toString(shape.getShapeType()));
    }
    return false;
}

bool CollisionHandler::rayShapeIntersect(const Shape& shape, const Ray& ray)
{
    return shapeRayIntersect(ray, shape);
}

bool CollisionHandler::shapeRayIntersect(const Shape& shape, const Ray& ray)
{
    return rayShapeIntersect(ray, shape);
}

bool CollisionHandler::shapeRayIntersect(const Ray& ray, const Shape& shape)
{
    return rayShapeIntersect(ray, shape);
}

bool CollisionHandler::shapeIntersect(const Shape& one, const Shape& two)
{
    // TODO:
    throw std::runtime_error("shapeIntersect(...) not implemented!");
}
