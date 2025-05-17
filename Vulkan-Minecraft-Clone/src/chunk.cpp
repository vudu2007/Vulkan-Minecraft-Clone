#include "chunk.hpp"

#include <algorithm>
#include <thread>

Chunk::Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size) : position(center_pos), size(size)
{
    const int x_start = static_cast<int>(center_pos.x - (size / static_cast<float>(2)));
    const int z_start = static_cast<int>(center_pos.y - (size / static_cast<float>(2)));

    int min_height = -25; // std::numeric_limits<int>::max(); // TODO
    for (int z = z_start; z < z_start + size; ++z)
    {
        for (int x = x_start; x < x_start + size; ++x)
        {
            int max_height = static_cast<int>(
                std::floorf(noise.getFractal2D(static_cast<float>(x), static_cast<float>(z), 5, 10.0f, 0.01f)));

            min_height = std::min(min_height, max_height);

            heightMap.emplace_back(glm::vec3(static_cast<float>(x), max_height, static_cast<float>(z)));
        }
    }

    blockMap = std::make_unique<std::unordered_map<glm::vec3, Block>>();
    unsigned counter = 0;
    for (int z = z_start; z < z_start + size; ++z)
    {
        for (int x = x_start; x < x_start + size; ++x)
        {
            const int max_height = static_cast<int>(heightMap[counter++].y);

            for (int y = min_height; y <= max_height; ++y)
            {
                const glm::vec3 position(static_cast<float>(x), y, static_cast<float>(z));
                blockMap->emplace(position, position);
            }
        }
    }

    for (auto& map : *blockMap)
    {
        Block& block = map.second;
        const glm::vec3 top = block.position + glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 bottom = block.position + glm::vec3(0.0f, -1.0f, 0.0f);
        const glm::vec3 left = block.position + glm::vec3(-1.0f, 0.0f, 0.0f);
        const glm::vec3 right = block.position + glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::vec3 front = block.position + glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 back = block.position + glm::vec3(0.0f, 0.0f, 1.0f);

        const bool has_top = blockMap->contains(top);
        const bool has_bottom = blockMap->contains(bottom);
        const bool has_left = blockMap->contains(left);
        const bool has_right = blockMap->contains(right);
        const bool has_front = blockMap->contains(front);
        const bool has_back = blockMap->contains(back);

        // if (has_top)
        //{
        //     block.topNeighbor = &((*blockMap)[top]);
        // }
        // if (has_bottom)
        //{
        //     block.topNeighbor = &(*blockMap)[bottom];
        // }
        // if (has_left)
        //{
        //     block.topNeighbor = &(*blockMap)[left];
        // }
        // if (has_right)
        //{
        //     block.topNeighbor = &(*blockMap)[right];
        // }
        // if (has_front)
        //{
        //     block.topNeighbor = &(*blockMap)[front];
        // }
        // if (has_back)
        //{
        //     block.topNeighbor = &(*blockMap)[back];
        // }

        if (has_top && has_bottom && has_left && has_right && has_front && has_back)
        {
            hiddenBlocks.push_back(&block);
        }
        else
        {
            visibleBlocks.push_back(&block);
        }
    }
}

void Chunk::merge(const std::vector<Chunk*>& others)
{
    std::list<std::list<Block*>::iterator> iterators;

    for (auto it = visibleBlocks.begin(); it != visibleBlocks.end(); ++it)
    {
        Block* block = *it;
        std::vector<glm::vec3> neighbors{
            (block->position + glm::vec3(0.0f, 1.0f, 0.0f)),  // Top.
            (block->position + glm::vec3(0.0f, -1.0f, 0.0f)), // Bottom.
            (block->position + glm::vec3(-1.0f, 0.0f, 0.0f)), // Left.
            (block->position + glm::vec3(1.0f, 0.0f, 0.0f)),  // Right.
            (block->position + glm::vec3(0.0f, 0.0f, -1.0f)), // Front.
            (block->position + glm::vec3(0.0f, 0.0f, 1.0f)),  // Back.
        };

        bool should_hide = true;
        for (const auto& neighbor : neighbors)
        {
            bool has_neighbor = false;

            // Check if this block has neighbors in other chunks.
            for (const auto& chunk : others)
            {
                if (chunk->blockMap->contains(neighbor))
                {
                    has_neighbor = true;
                    break;
                }
            }

            // Check if this block has neighbors in its own chunk.
            if (has_neighbor || blockMap->contains(neighbor))
            {
                continue;
            }

            // Didn't find any neighbors in own nor neighboring chunks, so this block remains visible.
            should_hide = false;
            break;
        }

        if (should_hide)
        {
            hiddenBlocks.push_back(block);
            iterators.push_back(it);
        }
    }

    for (const auto& it : iterators)
    {
        visibleBlocks.erase(it);
    }
}

glm::vec2 Chunk::getPos() const
{
    return position;
}

std::string Chunk::getPosStr() const
{
    return glm::to_string(position);
}

const std::vector<glm::vec3>& Chunk::getHeightMap() const
{
    return heightMap;
}

const std::list<Block*>& Chunk::getVisibleBlocks() const
{
    return visibleBlocks;
}
