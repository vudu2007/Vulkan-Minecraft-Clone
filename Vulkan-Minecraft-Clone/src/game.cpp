#include "game.hpp"
#include "world.hpp"

#include <iostream>
#include <unordered_map>

const int CHUNK_SIZE = 16;

void Game::run()
{
    World w{0, CHUNK_SIZE};
    w.updateChunks(player);
    auto chunks = w.getActiveChunks();

    player.addMoveCallback([&w](const Player& p) { w.updateChunks(p); });

    auto& height_map = chunks[0].getHeightMap();
    std::vector<Model::InstanceData> terrain;
    for (const auto& val : height_map)
    {
        terrain.emplace_back(val);
    }
    const unsigned terrain_v_buffer_idx = renderer.addVertexBuffer(
        renderer.pModel->getVertices().data(),
        sizeof(renderer.pModel->getVertices()[0]),
        renderer.pModel->getVertices().size(),
        renderer.pModel->getVertices().size(),
        terrain.data(),
        sizeof(terrain[0]),
        terrain.size(),
        terrain.size() * 1024); // TODO: adjust buffer size based on render distance; currently just a constant

    const unsigned uniform_buffer_idx = renderer.addUniformBuffer(0, sizeof(Model::UniformBufferObject));

    renderer.createDescriptorSets();
    while (!window.shouldClose())
    {
        window.pollEvents();

        // TODO: game logic here
        w.update(player);

        // Update uniforms.
        Model::UniformBufferObject ubo{};
        ubo.model = glm::identity<glm::mat4>();
        ubo.view = player.getCamera().viewMatrix();
        ubo.proj = player.getCamera().projMatrix();
        ubo.proj[1][1] *= -1;
        renderer.updateUniformBuffer(uniform_buffer_idx, &ubo, sizeof(ubo));

        terrain.clear();
        for (const auto& chunk : w.getActiveChunks())
        {
            auto& height_map = chunk.getHeightMap();
            for (const auto& val : height_map)
            {
                terrain.emplace_back(val);
            }
        }
        renderer.updateInstanceVertexBuffer(terrain_v_buffer_idx, terrain.data(), sizeof(terrain[0]), terrain.size());

        player.processInput();
        renderer.drawFrame();
    }
}
