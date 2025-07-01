#include "game.hpp"
#include "world.hpp"

#include <iostream>

struct LightingInfo
{
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 viewPos;
};

void Game::updateTerrain()
{
    const auto world_model = world.getModel();
    const auto& world_vertices = world_model.getVertices();
    const auto& world_indices = world_model.getIndices();
    renderer.updateVertexBuffer(
        terrainVertBufferIdx,
        world_vertices.data(),
        sizeof(world_vertices[0]),
        world_vertices.size());
    renderer
        .updateIndexBuffer(terrainVertBufferIdx, world_indices.data(), sizeof(world_indices[0]), world_indices.size());
}

void Game::run()
{
    Texture* block_texture_ptr = renderer.createTexture("src/textures/cube_texture.jpg");
    Model block_model("src/models/cube.obj");

    while (world.getActiveChunks().empty())
    {
    }

    const auto world_model = world.getModel();
    const auto& world_vertices = world_model.getVertices();
    const auto& world_indices = world_model.getIndices();
    terrainVertBufferIdx = renderer.addVertexBuffer(
        world_vertices.data(),
        sizeof(world_vertices[0]),
        world_vertices.size(),
        world_vertices.size() * 1024); // TODO
    renderer.createIndexBuffer(
        terrainVertBufferIdx,
        world_indices.data(),
        sizeof(world_indices[0]),
        world_indices.size(),
        world_indices.size() * 1024); // TODO

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
