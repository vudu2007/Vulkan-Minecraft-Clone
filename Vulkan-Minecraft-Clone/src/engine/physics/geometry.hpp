#pragma once

struct Geometry
{
    enum Type
    {
        SHAPE,
        RAY,
    };

    virtual Geometry::Type getGeometryType() const = 0;
};
