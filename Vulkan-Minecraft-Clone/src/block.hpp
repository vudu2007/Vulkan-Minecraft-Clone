#pragma once

#include "engine/physics/shapes/box.hpp"

#include "global.hpp"

struct Block
{
    glm::vec3 position;
    Box3d collisionShape;

    Block(const glm::vec3 position = glm::vec3(std::numeric_limits<float>::infinity()));

    const Box3d& getCollisionShape() const;
};
