#pragma once

#include "global.hpp"

struct Block
{
    glm::vec3 position;

    //Block* topNeighbor = nullptr;
    //Block* bottomNeighbor = nullptr;
    //Block* leftNeighbor = nullptr;
    //Block* rightNeighbor = nullptr;
    //Block* frontNeighbor = nullptr;
    //Block* backNeighbor = nullptr;
    //Block() = default;

    Block(const glm::vec3 position);
};
