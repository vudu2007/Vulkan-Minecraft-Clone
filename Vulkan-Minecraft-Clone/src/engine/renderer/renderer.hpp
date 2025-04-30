#ifndef VMC_SRC_ENGINE_RENDERER_RENDERER_HPP
#define VMC_SRC_ENGINE_RENDERER_RENDERER_HPP

#include "../../noise.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "texture.hpp"
#include "window.hpp"

#include <memory>

inline const int MAX_FRAMES_IN_FLIGHT = 2;

class Renderer
{
  private:
    uint32_t currentFrame = 0;
    Window& window;

    Device device;
    Swapchain swapchain;

    std::unique_ptr<DescriptorSetLayout> pDescriptorSetLayout;
    std::unique_ptr<DescriptorPool> pDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::unique_ptr<GraphicsPipeline> pGraphicsPipeline;
    VkPipelineLayout pipelineLayout;

    std::unique_ptr<Texture> pTexture;

    std::unique_ptr<Buffer> pIndexBuffer;

    struct VertexBufferInfo
    {
        size_t vertexCount = 0;
        size_t instanceCount = 0;
        std::unique_ptr<Buffer> pVertexBuffer;
        std::unique_ptr<Buffer> pInstanceVertexBuffer;
    };
    std::vector<VertexBufferInfo> vertexBufferPtrs;

    struct UniformBufferInfo
    {
        uint32_t binding;
        std::vector<std::unique_ptr<Buffer>> bufferPtrPerFrame;
    };
    std::vector<UniformBufferInfo> uniformBufferPtrs;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    void createDescriptorSetLayout();
    void createDescriptorPool();
    VkShaderModule createShaderModule(const std::vector<char>& bytecode) const;
    void createGraphicsPipeline();
    void createTextures();
    void loadModel();
    void createIndexBuffer();
    void createCommandBuffers();
    void createSyncObjects();

    void recordCommandBuffer(const VkCommandBuffer command_buffer, const uint32_t image_index);

  public:
    Renderer(Window& window);
    Renderer(const Renderer& other) = delete;
    Renderer(Renderer&& other) = delete;
    ~Renderer();

    Renderer& operator=(const Renderer& other) = delete;
    Renderer& operator=(Renderer&& other) = delete;

    std::unique_ptr<Model> pModel; // TODO: temp

    void createDescriptorSets();
    unsigned addVertexBuffer(
        const void* data,
        const size_t data_type_size,
        const size_t count,
        const size_t capacity,
        const void* instance_data = nullptr,
        const size_t instance_data_type_size = 0,
        const size_t instance_count = 0,
        const size_t instance_capacity = 0);
    void updateVertexBuffer(const unsigned index, const void* data, const size_t num_bytes);
    void updateInstanceVertexBuffer(
        const unsigned index,
        const void* data,
        const size_t data_type_size,
        const size_t count);

    unsigned addUniformBuffer(const uint32_t binding, const size_t byte_size);
    void updateUniformBuffer(const unsigned index, const void* data, const size_t byte_size);

    void drawFrame();
};

#endif // VMC_SRC_ENGINE_RENDERER_RENDERER_HPP
