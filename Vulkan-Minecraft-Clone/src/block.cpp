#include "block.hpp"

Block::Block(const glm::vec3 color) : color(color), collisionShape(glm::vec3(-0.5f), glm::vec3(0.5f))
{
}

const Aabb3d& Block::getCollisionShape() const
{
    return collisionShape;
}
