#include "game.hpp"
#include "world.hpp"

#include <iostream>

struct LightingInfo
{
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 viewPos;
};

void Game::generateTerrain()
{
    for (const auto& chunk : world.getActiveChunks())
    {
        auto& blockPositions = chunk.second->getVisibleBlockPositions();
        for (const auto& position : blockPositions)
        {
            terrain.emplace_back(position);
        }
    }
}

void Game::updateTerrain()
{
    terrain.clear();
    generateTerrain();
    renderer.updateInstanceVertexBuffer(terrainVBufferIdx, terrain.data(), sizeof(terrain[0]), terrain.size());
}

void Game::run()
{
    Texture* block_texture_ptr = renderer.createTexture("src/textures/cube_texture.jpg");
    Model block_model("src/models/cube.obj");

    while (world.getActiveChunks().empty())
    {
    }
    generateTerrain();

    terrainVBufferIdx = renderer.addVertexBuffer(
        block_model.getVertices().data(),
        sizeof(block_model.getVertices()[0]),
        block_model.getVertices().size(),
        block_model.getVertices().size(),
        terrain.data(),
        sizeof(terrain[0]),
        terrain.size(),
        terrain.size() * 1024); // TODO: adjust buffer size based on render distance; currently just a constant

    renderer.createIndexBuffer(
        terrainVBufferIdx,
        block_model.getIndices().data(),
        sizeof(block_model.getIndices()[0]),
        block_model.getIndices().size());

    const unsigned ubo_idx_transforms =
        renderer.addUniformBuffer(0, sizeof(Model::UniformBufferObject), VK_SHADER_STAGE_VERTEX_BIT);

    renderer.addCombinedImageSampler(1, block_texture_ptr, VK_SHADER_STAGE_FRAGMENT_BIT);

    LightingInfo ubo_lighting{};
    ubo_lighting.lightDir = glm::vec3(0.0f, 1.0f, 0.0f); // Direction to light.
    ubo_lighting.lightColor = glm::vec3(1.0f);
    ubo_lighting.viewPos = player.getPosition();
    const unsigned ubo_idx_light_info =
        renderer.addUniformBuffer(2, sizeof(ubo_lighting), VK_SHADER_STAGE_FRAGMENT_BIT);

    renderer.createDescriptorSetLayout();
    renderer.createGraphicsPipeline();
    renderer.createDescriptorSets();

    while (!window.shouldClose())
    {
        window.pollEvents();

        // TODO: game logic here

        // Update uniforms.
        Model::UniformBufferObject ubo{};
        ubo.model = glm::identity<glm::mat4>();
        ubo.view = player.getCamera().viewMatrix();
        ubo.proj = player.getCamera().projMatrix();
        ubo.proj[1][1] *= -1;
        renderer.updateUniformBuffer(ubo_idx_transforms, &ubo, sizeof(ubo));

        ubo_lighting.viewPos = player.getPosition();
        renderer.updateUniformBuffer(ubo_idx_light_info, &ubo_lighting, sizeof(ubo_lighting));

        // Update instance data.
        updateTerrain();

        player.processInput();
        renderer.drawFrame();
    }

    delete block_texture_ptr;
}
