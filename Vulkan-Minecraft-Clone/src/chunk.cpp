#include "chunk.hpp"

Chunk::Chunk(const SimplexNoise& noise, const glm::vec2& center_pos, const int size) : position(center_pos), size(size)
{
    const int x_start = static_cast<int>(center_pos.x - (size / static_cast<float>(2)));
    const int z_start = static_cast<int>(center_pos.y - (size / static_cast<float>(2)));

    for (int z = z_start; z < z_start + size; ++z)
    {
        for (int x = x_start; x < x_start + size; ++x)
        {
            heightMap.emplace_back(glm::vec3(
                static_cast<float>(x),
                noise.getFractal2D(static_cast<float>(x), static_cast<float>(z), 2, 10.0f, 0.01f),
                static_cast<float>(z)));
        }
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
