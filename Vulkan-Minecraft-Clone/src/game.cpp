#include "game.hpp"

#include <iostream>

void Game::run()
{
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
