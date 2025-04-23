#include "game.hpp"

#include <iostream>

SimplexNoise noise;

static std::vector<glm::vec3> generateTerrain()
{
    std::vector<glm::vec3> data;

    for (int z = 0; z < 64; ++z)
    {
        for (int x = 0; x < 64; ++x)
        {
            data.emplace_back(
                glm::vec3(static_cast<float>(x), noise.getFractal2D(x, z, 2, 10.0f, 0.01f), static_cast<float>(z)));
        }
    }

    return data;
}

void Game::run()
{
    auto height_map = generateTerrain();
    std::vector<Model::InstanceData> terrain;
    for (const auto& val : height_map)
    {
        terrain.emplace_back(val);
    }
    renderer.addVertexBuffer(
        renderer.pModel->getVertices().data(),
        sizeof(renderer.pModel->getVertices()[0]),
        renderer.pModel->getVertices().size(),
        terrain.data(),
        sizeof(terrain[0]),
        terrain.size());

    const unsigned uniformBufferIndex = renderer.addUniformBuffer(0, sizeof(Model::UniformBufferObject));

    renderer.createDescriptorSets();
    while (!window.shouldClose())
    {
        window.pollEvents();

        // TODO: game logic here

        // Update uniforms.
        Model::UniformBufferObject ubo{};
        ubo.model = glm::identity<glm::mat4>();
        ubo.view = camera.viewMatrix();
        ubo.proj = camera.projMatrix();
        ubo.proj[1][1] *= -1;
        renderer.updateUniformBuffer(uniformBufferIndex, &ubo, sizeof(ubo));

        camera.processInput();
        renderer.drawFrame();
    }
}
