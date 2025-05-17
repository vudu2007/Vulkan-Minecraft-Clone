#pragma once

#include "global.hpp"

#include <array>
#include <numbers>

/// <summary>
/// Currently, based on Stefan Gustavson's implementation of simplex noise
/// https://github.com/stegu/perlin-noise/blob/master/simplexnoise.pdf
/// </summary>
class SimplexNoise
{
  private:
    static const unsigned NUM_RANDOM_VALUES = 256;
    static const unsigned NUM_PERMUTATIONS = 512;

    static const unsigned DEFAULT_SEED = 0;
    inline static const float DEFAULT_RADIUS = static_cast<float>(std::sin(std::numbers::pi / 4.0)); // 0.5 if squared
    inline static const float DEFAULT_AMPLITUDE_MULTIPLIER = 0.5f;
    inline static const float DEFAULT_FREQUENCY_MULTIPLIER = 2.0f;

    const unsigned seed;
    const float radius; // Radius of kernels.

    std::array<glm::vec2, 4> gradients2D{
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, -1.0f),
        glm::vec2(-1.0f, 1.0f),
        glm::vec2(-1.0f, -1.0f),
    };
    std::array<unsigned, NUM_RANDOM_VALUES> randomValues{};
    std::array<unsigned, NUM_PERMUTATIONS> permutations{};

  public:
    SimplexNoise(const unsigned seed = DEFAULT_SEED, const float radius = DEFAULT_RADIUS);

    float get2D(const float x, const float y) const;
    float getFractal2D(
        const float x,
        const float y,
        const unsigned int octaves,
        float amplitude,
        float frequency,
        const float amplitude_mult = DEFAULT_AMPLITUDE_MULTIPLIER,
        const float frequency_mult = DEFAULT_FREQUENCY_MULTIPLIER) const;
};
