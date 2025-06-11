#pragma once

#include "block.hpp"
#include "noise.hpp"
#include "player.hpp"

#include <GLM/gtx/hash.hpp>

#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Chunk
{
  private:
    using BlockId = glm::vec3;

    glm::vec2 position;
    int size;

    std::unique_ptr<std::unordered_map<BlockId, Block>> blockMap;
    std::list<Block*> visibleBlocks;
    // std::list<Block*> hiddenBlocks; // TODO
    std::list<BlockId> hiddenBlocks;

  public:
    Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size);

    const Block* getBlockInReach(const Player& player) const;

    glm::vec2 getPos() const;
    std::string getPosStr() const;

    const std::list<Block*>& getVisibleBlocks() const;
};
