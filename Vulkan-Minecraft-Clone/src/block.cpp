#include "block.hpp"

Block::Block(const glm::vec3 position)
    : position(position), collisionShape(position + glm::vec3(-0.5f), position + glm::vec3(0.5f))
{
}

const Box3d& Block::getCollisionShape() const
{
    return collisionShape;
}
