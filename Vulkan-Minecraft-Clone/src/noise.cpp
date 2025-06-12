#include "noise.hpp"

#include <cmath>
#include <iostream>
#include <random>
#include <ranges>

SimplexNoise::SimplexNoise(const unsigned seed, const float radius) : seed(seed), radius(radius)
{
    // Generate an array with values of its indices and shuffle them.
    std::mt19937 rand_num_gen(seed);
    for (size_t i = 0; i < randomValues.size(); ++i)
    {
        randomValues[i] = static_cast<unsigned>(i);
    }
    std::ranges::shuffle(randomValues, rand_num_gen);

    for (size_t i = 0; i < permutations.size(); ++i)
    {
        permutations[i] = randomValues[i & (randomValues.size() - 1)];
    }
}

float SimplexNoise::get2D(const float x, const float y) const
{
    const unsigned n = 2; // Number of coordinates.
    const glm::vec2 pos(x, y);

    // 1. Coordinate skewing.
    // Determine which simplex the coordinates lie in.
    const float skew_val = (std::sqrtf(static_cast<float>(n) + 1.0f) - 1.0f) / static_cast<float>(n);
    const glm::vec2 skewed_pos = pos + glm::vec2((x + y) * skew_val);

    // 2. Simplicial division.
    // Create simplex by inserting skewed coordinates in decreasing order.
    // If x value is larger than y, then we get vertices order (0, 0), (1, 0), (1, 1),
    // otherwise, y is larger than x, so (0, 0), (0, 1), (1, 1).
    const glm::vec2 skewed_cell_pos = glm::floor(skewed_pos);
    const glm::vec2 internal_skewed_pos = skewed_pos - skewed_cell_pos;

    const glm::vec2 v_0(0);
    const glm::vec2 v_1 = (internal_skewed_pos.x > internal_skewed_pos.y) ? glm::vec2(1, 0) : glm::vec2(0, 1);
    const glm::vec2 v_2(1);

    // 3. Gradient selection.
    const unsigned max_random_value = static_cast<unsigned>(randomValues.size()) - 1;

    const glm::vec2 floor_v_0 = glm::floor(v_0 + skewed_cell_pos);
    const glm::vec2 floor_v_1 = glm::floor(v_1 + skewed_cell_pos);
    const glm::vec2 floor_v_2 = glm::floor(v_2 + skewed_cell_pos);

    const glm::uvec2 constr_floor_v_0(
        static_cast<int>(floor_v_0.x) & max_random_value,
        static_cast<int>(floor_v_0.y) & max_random_value);
    const glm::uvec2 constr_floor_v_1(
        static_cast<int>(floor_v_1.x) & max_random_value,
        static_cast<int>(floor_v_1.y) & max_random_value);
    const glm::uvec2 constr_floor_v_2(
        static_cast<int>(floor_v_2.x) & max_random_value,
        static_cast<int>(floor_v_2.y) & max_random_value);

    std::array<glm::vec2, 3> gradients{};
    gradients[0] =
        gradients2D[permutations[constr_floor_v_0.x + permutations[constr_floor_v_0.y]] % gradients2D.size()];
    gradients[1] =
        gradients2D[permutations[constr_floor_v_1.x + permutations[constr_floor_v_1.y]] % gradients2D.size()];
    gradients[2] =
        gradients2D[permutations[constr_floor_v_2.x + permutations[constr_floor_v_2.y]] % gradients2D.size()];

    // 4. Kernel summation.
    // Obtain the unskewed point.
    const float unskew_val = (1.0f - 1.0f / (std::sqrtf(static_cast<float>(n) + 1.0f))) / static_cast<float>(n);

    const glm::vec2 unskewed_cell_pos = skewed_cell_pos - ((skewed_cell_pos.x + skewed_cell_pos.y) * unskew_val);

    std::array<glm::vec2, 3> displacements{};
    displacements[0] = pos - unskewed_cell_pos;
    displacements[1] = displacements[0] - v_1 + ((v_1.x + v_1.y) * unskew_val);
    displacements[2] = displacements[0] - v_2 + ((v_2.x + v_2.y) * unskew_val);

    // Calculate contributions of each vertex.
    // t = r^2 - d^2, where `d` is the distance to the unskewed point.
    // t = r^2 - ((x1 - x0) + (y1 - y0)) = r^2 - dx - dy
    float total_contrib = 0.0f;
    const float r_squared = radius * radius;
    for (unsigned i = 0; i < 3; ++i)
    {
        const float d_squared = displacements[i].x * displacements[i].x + displacements[i].y * displacements[i].y;
        float t = std::max(0.0f, r_squared - d_squared);
        t *= t;
        total_contrib += t * t * glm::dot(displacements[i], gradients[i]);
    }

    // TODO: haven't figured out how the scale (70.0f) was calculated
    return 70.0f * total_contrib;
}

float SimplexNoise::getFractal2D(
    const float x,
    const float y,
    const unsigned int octaves,
    float amplitude,
    float frequency,
    const float amplitude_mult,
    const float frequency_mult) const
{
    float result = 0.0f;

    for (unsigned i = 0; i < octaves; ++i)
    {
        result += amplitude * get2D(x * frequency, y * frequency);
        amplitude *= amplitude_mult;
        frequency *= frequency_mult;
    }

    return result;
}
