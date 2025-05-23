#include "game.hpp"
#include "world.hpp"

#include <iostream>

const int CHUNK_SIZE = 16;

struct LightingInfo
{
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 viewPos;
};

void Game::run()
{
    Texture* viking_texture_ptr = renderer.createTexture("src/textures/viking_room.png");
    Model viking_model("src/models/viking_room.obj", 5.0f);

    std::vector<Model::InstanceData> viking_instance_data;
    viking_instance_data.emplace_back(glm::vec3(0.0f, 15.0f, 0.0f));
    const unsigned viking_v_buffer_idx = renderer.addVertexBuffer(
        viking_model.getVertices().data(),
        sizeof(viking_model.getVertices()[0]),
        viking_model.getVertices().size(),
        viking_model.getVertices().size(),
        viking_instance_data.data(),
        sizeof(viking_instance_data[0]),
        viking_instance_data.size(),
        viking_instance_data.size());

    renderer.createIndexBuffer(
        viking_v_buffer_idx,
        viking_model.getIndices().data(),
        sizeof(viking_model.getIndices()[0]),
        viking_model.getIndices().size());

    //////////////////////////////

    Texture* block_texture_ptr = renderer.createTexture("src/textures/cube_texture.jpg");
    Model block_model("src/models/cube.obj");

    const unsigned seed = 727;
    World w{seed, CHUNK_SIZE, player};

    player.addMoveCallback([&w](const Player& p) { w.updateChunks(p); });

    std::vector<Model::InstanceData> terrain;
    auto& blocks = (w.getActiveChunks())[0]->getVisibleBlocks();
    for (const auto& block : blocks)
    {
        terrain.emplace_back(block->position);
    }

    const unsigned terrain_v_buffer_idx = renderer.addVertexBuffer(
        block_model.getVertices().data(),
        sizeof(block_model.getVertices()[0]),
        block_model.getVertices().size(),
        block_model.getVertices().size(),
        terrain.data(),
        sizeof(terrain[0]),
        terrain.size(),
        terrain.size() * 10240); // TODO: adjust buffer size based on render distance; currently just a constant

    renderer.createIndexBuffer(
        terrain_v_buffer_idx,
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
        w.update(player);

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

    delete block_texture_ptr;
    delete viking_texture_ptr;
}
