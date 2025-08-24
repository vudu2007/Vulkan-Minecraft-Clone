#pragma once

struct Geometry
{
    enum class Type
    {
        SHAPE,
        RAY,
    };

    virtual Geometry::Type getGeometryType() const = 0;
};
