#pragma once

#include "../window.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "device.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"
#include "texture.hpp"

#include <memory>

inline const int MAX_FRAMES_IN_FLIGHT = 2;

class Renderer
{
  private:
    uint32_t currentFrame = 0;
    Window& window;

    Device device;
    Swapchain swapchain;

    // Descriptors.
    std::unique_ptr<DescriptorSetLayout> pDescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    std::unique_ptr<DescriptorPool> pDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // Pipelines.
    std::unique_ptr<GraphicsPipeline> pGraphicsPipeline;
    VkPipelineLayout pipelineLayout;

    // Buffers.
    struct IndexBufferInfo
    {
        size_t count = 0;
        std::unique_ptr<Buffer> pBuffer;
        VkIndexType type = VK_INDEX_TYPE_UINT32; // TODO

        IndexBufferInfo() = default;
        IndexBufferInfo(size_t count, std::unique_ptr<Buffer> pBuffer, VkIndexType type = VK_INDEX_TYPE_UINT32);
        IndexBufferInfo(const IndexBufferInfo& other);
        IndexBufferInfo(IndexBufferInfo&& other) noexcept;
    };
    using IndexBuffers = std::unordered_map<unsigned, IndexBufferInfo>;
    std::unordered_map<unsigned, IndexBuffers> vertToIndexBuffers;

    // TODO: need a away to allow for drawing vertices without indices if not specified.
    struct VertexBufferInfo
    {
        size_t vertexCount = 0;
        size_t instanceCount = 1;
        std::unique_ptr<Buffer> pVertexBuffer;
        std::unique_ptr<Buffer> pInstanceVertexBuffer;
    };
    std::unordered_map<unsigned, VertexBufferInfo> vertexBuffers;

    struct UniformBufferInfo
    {
        uint32_t binding;
        std::vector<std::unique_ptr<Buffer>> bufferPtrPerFrame;

        UniformBufferInfo(const uint32_t binding, std::vector<std::unique_ptr<Buffer>>&& buffer_ptr_per_frame);
    };
    std::vector<UniformBufferInfo> uniformBuffers;

    struct CombinedImageSamplerInfo
    {
        uint32_t binding;
        const Texture* texture;
    };
    std::vector<CombinedImageSamplerInfo> combinedImageSamplers;

    std::vector<VkCommandBuffer> commandBuffers;

    // Synchronization primitives.
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    VkShaderModule createShaderModule(const std::vector<char>& bytecode) const;
    void createDescriptorPool();
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

    [[nodiscard]] Texture* createTexture(const std::string& path) const;

    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createDescriptorSets();

    bool addVertexBuffer(
        const unsigned id,
        const void* data,
        const size_t data_type_size,
        const size_t count,
        const size_t capacity,
        const void* instance_data = nullptr,
        const size_t instance_data_type_size = 0,
        const size_t instance_count = 1,
        const size_t instance_capacity = 1);
    bool updateVertexBuffer(const unsigned id, const void* data, const size_t data_type_size, const size_t count);
    bool updateInstanceVertexBuffer(
        const unsigned id,
        const void* data,
        const size_t data_type_size,
        const size_t count);

    bool addIndexBuffer(
        const unsigned vertex_buffer_id,
        const unsigned index_buffer_id,
        const void* data,
        const size_t data_type_size,
        const size_t count,
        const size_t capacity);
    bool updateIndexBuffer(
        const unsigned vertex_buffer_id,
        const unsigned index_buffer_id,
        const void* data,
        const size_t data_type_size,
        const size_t count);

    void removeVertexBuffer(const unsigned id);
    void removeIndexBuffer(const unsigned vertex_buffer_id, const unsigned index_buffer_id);

    // unsigned addUniformBufferArray();
    unsigned addUniformBuffer(
        const uint32_t binding,
        const size_t byte_size,
        const VkShaderStageFlagBits stage_flags,
        const uint32_t array_size = 1);
    void updateUniformBuffer(const unsigned index, const void* data, const size_t byte_size);

    // void addCombinedImageSamplerArray();
    void addCombinedImageSampler(
        const uint32_t binding,
        const Texture* texture,
        const VkShaderStageFlagBits stage_flags,
        const uint32_t array_size = 1,
        const VkSampler* immutable_samplers = nullptr);

    void drawFrame();
};
