#include "block.hpp"

Block::Block(const glm::vec3 position, const glm::vec3 color)
    : position(position), color(color), collisionShape(position + glm::vec3(-0.5f), position + glm::vec3(0.5f))
{
}

const Box3d& Block::getCollisionShape() const
{
    return collisionShape;
}
