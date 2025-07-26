#pragma once

#include "engine/physics/shapes/box.hpp"

#include "global.hpp"

struct Block
{
    glm::vec3 color;
    Box3d collisionShape;

    Block(const glm::vec3 color = glm::vec3(0.0f));

    const Box3d& getCollisionShape() const;
};
