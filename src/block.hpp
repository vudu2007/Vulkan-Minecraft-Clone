#pragma once

#include "engine/physics/shapes/aabb.hpp"

struct Block
{
    glm::vec3 color;
    Aabb3d collisionShape;

    Block(const glm::vec3 color = glm::vec3(0.0f));

    const Aabb3d& getCollisionShape() const;
};
