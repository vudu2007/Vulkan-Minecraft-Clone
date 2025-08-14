#include "game.hpp"
#include "world.hpp"

#include <chrono>
#include <iostream>

struct LightingInfo
{
    alignas(16) glm::vec3 lightDir;
    alignas(16) glm::vec3 lightColor;
    alignas(16) glm::vec3 viewPos;
};

void Game::loadChunkModel(const Chunk& chunk)
{
    const ChunkCenter cc = chunk.getCenter();
    const Model& chunk_model = chunk.getModel();

    const auto& chunk_vertices = chunk_model.getVertices();
    const auto& chunk_indices = chunk_model.getIndices();

    std::unique_lock<std::mutex> lock(updateMutex);

    // Handle an empty (no model) chunk.
    if (chunk_vertices.empty() || chunk_indices.empty())
    {
        // If this chunk existed before, we need to unload it.
        if (chunkToVertexBufferId.contains(cc))
        {
            lock.unlock();
            unloadChunkModel(chunk);
        }
        return;
    }

    if (chunkToVertexBufferId.contains(cc)) // Chunk already present, so update it.
    {
        const unsigned id = chunkToVertexBufferId[cc];

        renderer.updateVertexBuffer(
            chunkToVertexBufferId[cc],
            chunk_vertices.data(),
            sizeof(chunk_vertices[0]),
            chunk_vertices.size());

        renderer.updateIndexBuffer(id, 0, chunk_indices.data(), sizeof(chunk_indices[0]), chunk_indices.size());
    }
    else // Chunk not present, so add it.
    {
        // Figure out vertex buffer id.
        unsigned id;
        if (reusableIds.empty())
        {
            id = chunkToVertexBufferId.size();
        }
        else
        {
            id = reusableIds.back();
            reusableIds.erase(reusableIds.end() - 1);
        }
        chunkToVertexBufferId[cc] = id;

        // Max number of indices = MAX_NUM_BLOCKS_IN_CHUNK * 6 faces per block * 2 triangles per face * 3 vertices per
        // triangle
        renderer.addVertexBuffer(
            id,
            chunk_vertices.data(),
            sizeof(chunk_vertices[0]),
            chunk_vertices.size(),
            static_cast<size_t>(MAX_NUM_BLOCKS_IN_CHUNK * 36));

        // Max number of indices = MAX_NUM_BLOCKS_IN_CHUNK * 6 faces per block * 6 indices per face
        renderer.addIndexBuffer(
            id,
            0,
            chunk_indices.data(),
            sizeof(chunk_indices[0]),
            chunk_indices.size(),
            static_cast<size_t>(MAX_NUM_BLOCKS_IN_CHUNK) * 36);
    }
}

void Game::unloadChunkModel(const Chunk& chunk)
{
    const ChunkCenter cc = chunk.getCenter();

    std::lock_guard<std::mutex> lock(updateMutex);

    if (!chunkToVertexBufferId.contains(cc))
    {
        return; // Ignore this chunk since it isn't loaded, e.g. empty chunks.
    }

    const unsigned id = chunkToVertexBufferId[cc];
    renderer.removeVertexBuffer(id);
    chunkToVertexBufferId.erase(cc);
    reusableIds.push_back(id);
}

void Game::run()
{
    Texture* block_texture_ptr = renderer.createTexture("../../Vulkan-Minecraft-Clone/src/textures/cube_texture.jpg");

    world.addChunkLoadedCallback([this](const Chunk& chunk) { loadChunkModel(chunk); });
    world.addChunkUnloadedCallback([this](const Chunk& chunk) { unloadChunkModel(chunk); });
    world.init(DEFAULT_PLAYER_POS, DEFAULT_PLAYER_RENDER_DISTANCE);
    std::cout << "Number of vertex buffers in use = " << chunkToVertexBufferId.size() << std::endl;

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

    std::chrono::steady_clock::time_point last_frame_time = std::chrono::steady_clock::now();
    double accum_time = 0.0;
    while (!window.shouldClose())
    {
        window.pollEvents();

        // TODO: game logic here
        std::chrono::steady_clock::time_point curr_frame_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> delta_time = curr_frame_time - last_frame_time;
        last_frame_time = curr_frame_time;
        accum_time += delta_time.count();
        if (accum_time >= 0.3)
        {
            accum_time = 0.0;
            double fps = 1.0 / delta_time.count();
            // std::cout << "\rFPS: " << static_cast<int>(fps) << "     ";
        }

        // Update uniforms.
        Model::UniformBufferObject ubo{};
        ubo.model = glm::identity<glm::mat4>();
        ubo.view = player.getCamera().viewMatrix();
        ubo.proj = player.getCamera().projMatrix();
        ubo.proj[1][1] *= -1;
        renderer.updateUniformBuffer(ubo_idx_transforms, &ubo, sizeof(ubo));

        ubo_lighting.viewPos = player.getPosition();
        renderer.updateUniformBuffer(ubo_idx_light_info, &ubo_lighting, sizeof(ubo_lighting));

        player.update(delta_time.count());

        {
            std::lock_guard<std::mutex> lock(updateMutex);
            renderer.drawFrame();
        }
    }

    delete block_texture_ptr;
}
