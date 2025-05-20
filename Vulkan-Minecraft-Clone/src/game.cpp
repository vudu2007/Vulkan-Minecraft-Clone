#include "game.hpp"
#include "world.hpp"

#include <iostream>

const int CHUNK_SIZE = 16;

void Game::run()
{
    World w{0, CHUNK_SIZE, player};

    player.addMoveCallback([&w](const Player& p) { w.updateChunks(p); });

    std::vector<Model::InstanceData> terrain;
    auto& blocks = (w.getActiveChunks())[0]->getVisibleBlocks();
    for (const auto& block : blocks)
    {
        terrain.emplace_back(block->position);
    }

    const unsigned terrain_v_buffer_idx = renderer.addVertexBuffer(
        renderer.pModel->getVertices().data(),
        sizeof(renderer.pModel->getVertices()[0]),
        renderer.pModel->getVertices().size(),
        renderer.pModel->getVertices().size(),
        terrain.data(),
        sizeof(terrain[0]),
        terrain.size(),
        terrain.size() * 10240); // TODO: adjust buffer size based on render distance; currently just a constant

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
            auto& blocks = chunk->getVisibleBlocks();
            for (const auto& block : blocks)
            {
                terrain.emplace_back(block->position);
            }
        }
        renderer.updateInstanceVertexBuffer(terrain_v_buffer_idx, terrain.data(), sizeof(terrain[0]), terrain.size());

        player.processInput();
        renderer.drawFrame();
    }
}
