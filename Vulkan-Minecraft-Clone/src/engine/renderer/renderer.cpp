#include "renderer.hpp"

#include "../../utility.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>

void Renderer::createDescriptorSetLayout()
{
    pDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(device, descriptorSetLayoutBindings);
}

void Renderer::createDescriptorPool()
{
    // TODO: will need to test
    const size_t num_buffers = uniformBuffers.size();
    const size_t num_samplers = 1;

    std::vector<VkDescriptorPoolSize> pool_sizes;

    for (size_t i = 0; i < num_buffers; ++i)
    {
        pool_sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)});
    }
    for (size_t i = 0; i < num_samplers; ++i)
    {
        pool_sizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)});
    }

    pDescriptorPool = std::make_unique<DescriptorPool>(device, pool_sizes, static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
}

void Renderer::createDescriptorSets()
{
    // Create the descriptor pool to allocate the sets.
    createDescriptorPool();

    // TODO: will need to test
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pDescriptorSetLayout->getLayout());
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pDescriptorPool->getPool();
    alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    alloc_info.pSetLayouts = layouts.data();

    descriptorSets = pDescriptorPool->allocateDescriptorSets(*pDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT);

    const size_t num_buffers = uniformBuffers.size();
    std::vector<std::pair<uint32_t, std::vector<VkDescriptorBufferInfo>>> buffer_infos;

    // Uniform buffers.
    for (size_t i = 0; i < uniformBuffers.size(); ++i)
    {
        std::vector<VkDescriptorBufferInfo> buffer_infos_per_frame;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j)
        {
            VkDescriptorBufferInfo buffer_info_per_frame{};
            buffer_info_per_frame.buffer = uniformBuffers[i].bufferPtrPerFrame[j]->getBuffer();
            buffer_info_per_frame.range = uniformBuffers[i].bufferPtrPerFrame[j]->getSize();
            buffer_infos_per_frame.push_back(buffer_info_per_frame);
        }
        buffer_infos.push_back(std::make_pair(uniformBuffers[i].binding, buffer_infos_per_frame));
    }

    const size_t num_images = combinedImageSamplers.size();
    std::vector<std::pair<uint32_t, std::vector<VkDescriptorImageInfo>>> image_infos;

    // Combined Image Samplers.
    for (size_t i = 0; i < combinedImageSamplers.size(); ++i)
    {
        std::vector<VkDescriptorImageInfo> image_infos_per_frame{};
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j)
        {
            VkDescriptorImageInfo image_info{};
            image_info.sampler = combinedImageSamplers[i].texture->getSampler();
            image_info.imageView = combinedImageSamplers[i].texture->getImageView();
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_infos_per_frame.push_back(image_info);
        }
        image_infos.push_back(std::make_pair(combinedImageSamplers[i].binding, image_infos_per_frame));
    }

    // Update the descriptor sets.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        size_t offset = 0;
        std::vector<VkWriteDescriptorSet> descriptor_writes(num_images + num_buffers);

        for (size_t j = 0; j < image_infos.size(); ++j)
        {
            const auto binding = image_infos[j].first;
            const auto& sampler_info = image_infos[j].second[i];
            descriptor_writes[j + offset].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[j + offset].dstSet = descriptorSets[i];
            descriptor_writes[j + offset].dstBinding = binding;
            descriptor_writes[j + offset].dstArrayElement = 0;
            descriptor_writes[j + offset].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_writes[j + offset].descriptorCount = 1;
            descriptor_writes[j + offset].pImageInfo = &sampler_info;
        }
        offset += image_infos.size();

        for (size_t j = 0; j < buffer_infos.size(); ++j)
        {
            const auto binding = buffer_infos[j].first;
            const auto& buffer_info = buffer_infos[j].second[i];
            descriptor_writes[j + offset].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_writes[j + offset].dstSet = descriptorSets[i];
            descriptor_writes[j + offset].dstBinding = binding;
            descriptor_writes[j + offset].dstArrayElement = 0;
            descriptor_writes[j + offset].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_writes[j + offset].descriptorCount = 1;
            descriptor_writes[j + offset].pBufferInfo = &buffer_info;
        }
        offset += buffer_infos.size();

        pDescriptorPool->updateDescriptorSets(descriptor_writes);
    }
}

unsigned Renderer::addVertexBuffer(
    const void* data,
    const size_t data_type_size,
    const size_t count,
    const size_t capacity,
    const void* instance_data,
    const size_t instance_data_type_size,
    const size_t instance_count,
    const size_t instance_capacity)
{
    // Handle per vertex data.
    size_t num_bytes_capacity = data_type_size * capacity;
    size_t num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = num_bytes_capacity;
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Buffer staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    staging_buffer.map();
    staging_buffer.write(data, num_bytes);
    staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    // Create the vertex buffer and copy the data from the staging buffer into it.
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vertexBuffers.push_back(
        {count,
         instance_count,
         std::make_unique<Buffer>(
             device,
             create_info,
             VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
             VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT),
         nullptr});
    vertexBuffers.back().pVertexBuffer->copyFrom(staging_buffer, num_bytes);

    // Check if there is per instance data to handle.
    if (instance_data == nullptr)
    {
        return static_cast<unsigned>(vertexBuffers.size() - 1);
    }

    // Handle per instance data if it was provided.
    num_bytes_capacity = instance_data_type_size * instance_capacity;
    num_bytes = instance_data_type_size * instance_count;

    // Make a staging buffer so that the host can write to it.
    create_info.size = num_bytes_capacity;
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    Buffer instance_staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    instance_staging_buffer.map();
    instance_staging_buffer.write(instance_data, num_bytes);
    instance_staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    // Create the vertex buffer and copy the data from the staging buffer into it.
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vertexBuffers.back().instanceCount = instance_count;
    vertexBuffers.back().pInstanceVertexBuffer = std::make_unique<Buffer>(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
    vertexBuffers.back().pInstanceVertexBuffer->copyFrom(instance_staging_buffer, num_bytes);

    return static_cast<unsigned>(vertexBuffers.size() - 1);
}

void Renderer::updateVertexBuffer(
    const unsigned index,
    const void* data,
    const size_t data_type_size,
    const size_t count)
{
    vertexBuffers[index].vertexCount = count;
    if (count == 0)
    {
        return;
    }

    size_t num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = num_bytes;
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Buffer staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    staging_buffer.map();
    staging_buffer.write(data, num_bytes);
    staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    // Create the vertex buffer and copy the data from the staging buffer into it.
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // TODO: do double buffering.
    // TODO: may need synchronization primitive to render only after update
    vertexBuffers[index].pVertexBuffer->copyFrom(staging_buffer, num_bytes);
}

void Renderer::updateInstanceVertexBuffer(
    const unsigned index,
    const void* data,
    const size_t data_type_size,
    const size_t count)
{
    size_t num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = num_bytes;
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Buffer instance_staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    instance_staging_buffer.map();
    instance_staging_buffer.write(data, num_bytes);
    instance_staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    // Create the vertex buffer and copy the data from the staging buffer into it.
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // TODO: do double buffering.
    // TODO: may need synchronization primitive to render only after update
    vertexBuffers[index].instanceCount = count;
    vertexBuffers[index].pInstanceVertexBuffer->copyFrom(instance_staging_buffer, num_bytes);
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& bytecode) const
{
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = bytecode.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device.getLogicalDevice(), &create_info, nullptr, &shader_module))
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shader_module;
}

void Renderer::createGraphicsPipeline()
{
    // Load the shaders.
    auto vert_shader_code = VmcUtility::readFile("src/shaders/shader_block_vert.spv");
    auto frag_shader_code = VmcUtility::readFile("src/shaders/shader_block_frag.spv");
    VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
    VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

    // Handle programmable shaders for pipeline.
    // Vertex.
    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";                // Entrypoint.
    vert_shader_stage_info.pSpecializationInfo = nullptr; // Optional. Specify shader constants.

    // Fragment.
    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main"; // Entrypoint.

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

    // Handle fixed stages and other things.
    // Dynamic states are states that can be changed in the fixed pipeline at
    // draw time.
    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    // Describe vertex data.
    std::vector<VkVertexInputBindingDescription> binding_descriptions{
        Model::Vertex::getBindingDescription(),
    };
    const auto& attribute_description_vertex = Model::Vertex::getAttributeDescriptions();
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    attribute_descriptions.insert(
        attribute_descriptions.end(),
        attribute_description_vertex.begin(),
        attribute_description_vertex.end());

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    // Input assembly.
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissors.
    // Viewport is the transformation from an image to the framebuffer.
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.getExtent().width);
    viewport.height = static_cast<float>(swapchain.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor is the crop tool but does not resize the viewport image.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.getExtent();

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = nullptr; // Set only if not dynamic.
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = nullptr; // Set only if not dynamic.

    // Rasterizer.
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;        // Clamp fragments beyond planes to planes.
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // Geometry never passes through rasterizer stage.
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Detemines how fragments are generated for
                                                   // geometry.
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE; // Alter depth values by bias.
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling.
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = device.getMsaaSamples();
    multisampling.sampleShadingEnable = VK_FALSE; // Enable sample shading feature for the device.
    multisampling.minSampleShading = 0.2f;        // Min. fraction for sample shading; closer to one is smoother.
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing.
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f; // Optional.
    depth_stencil.maxDepthBounds = 1.0f; // Optional.
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {}; // Optional.
    depth_stencil.back = {};  // Optional.

    // Color blending.
    // Attachment is config per attached framebuffer while create info is
    // global.
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional.
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional.
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional.
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional.
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional.
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional.

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional.
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional.
    color_blending.blendConstants[1] = 0.0f; // Optional.
    color_blending.blendConstants[2] = 0.0f; // Optional.
    color_blending.blendConstants[3] = 0.0f; // Optional.

    // Pipeline layout (+ uniforms).
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    VkDescriptorSetLayout descriptor_set_layout = pDescriptorSetLayout->getLayout();
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;                   // Optional.
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout; // Optional.
    pipeline_layout_info.pushConstantRangeCount = 0;           // Optional.
    pipeline_layout_info.pPushConstantRanges = nullptr;        // Optional.

    if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipeline_layout_info, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // Create the graphics pipeline.
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // Shader stages.
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;

    // Fixed stages.
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;

    // Layout.
    pipeline_info.layout = pipelineLayout;

    // Render passes.
    pipeline_info.renderPass = swapchain.getRenderPass();
    pipeline_info.subpass = 0; // Index of subpass.

    // Pipeline derivative.
    // "These values are only used if the `VK_PIPELINE_CREATE_DERIVATIVE_BIT` flag is also specified in the flags
    // field of `VkGraphicsPipelineCreateInfo`."
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional.
    pipeline_info.basePipelineIndex = -1;              // Optional.

    pGraphicsPipeline = std::make_unique<GraphicsPipeline>(device, pipeline_info);

    // Clean up.
    vkDestroyShaderModule(device.getLogicalDevice(), frag_shader_module, nullptr);
    vkDestroyShaderModule(device.getLogicalDevice(), vert_shader_module, nullptr);
}

[[nodiscard]] Texture* Renderer::createTexture(const std::string& path) const
{
    return new Texture(device, path);
}

void Renderer::createIndexBuffer(
    const unsigned vertex_buffer_index,
    const void* data,
    const size_t data_type_size,
    const size_t count,
    const size_t capacity)
{
    const size_t num_bytes_capacity = data_type_size * capacity;
    const VkDeviceSize num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = num_bytes_capacity;
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Buffer staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    staging_buffer.map();
    staging_buffer.write(data, num_bytes);
    staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    // Create the index buffer and copy the data from the staging buffer into it.
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBuffers.push_back(
        {vertex_buffer_index,
         count,
         std::make_unique<Buffer>(
             device,
             create_info,
             VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
             VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)});
    indexBuffers.back().pBuffer->copyFrom(staging_buffer, num_bytes);
}

void Renderer::updateIndexBuffer(
    const unsigned vertex_buffer_index,
    const void* data,
    const size_t data_type_size,
    const size_t count)
{
    indexBuffers[vertex_buffer_index].count = count;
    if (count == 0)
    {
        return;
    }

    const VkDeviceSize num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = num_bytes;
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    Buffer staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    staging_buffer.map();
    staging_buffer.write(data, num_bytes);
    staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    // Create the index buffer and copy the data from the staging buffer into it.
    create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    indexBuffers[vertex_buffer_index].count = count;
    indexBuffers[vertex_buffer_index].pBuffer->copyFrom(staging_buffer, num_bytes);
}

void Renderer::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = device.getCommandPool();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(device.getLogicalDevice(), &alloc_info, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    const VkDevice& logical_device = device.getLogicalDevice();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if ((vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) ||
            (vkCreateFence(logical_device, &fence_info, nullptr, &inFlightFences[i]) != VK_SUCCESS))
        {
            throw std::runtime_error("failed to create synchronization objects!");
        }
    }
}

void Renderer::recordCommandBuffer(const VkCommandBuffer command_buffer, const uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;                  // Optional.
    begin_info.pInheritanceInfo = nullptr; // Optional.

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Start a render pass.
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = swapchain.getRenderPass();
    render_pass_info.framebuffer = swapchain.getFramebuffers()[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain.getExtent();

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {
        {0.43f, 0.7f, 0.92f, 1.0f}
    };
    clear_values[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.getExtent().width);
    viewport.height = static_cast<float>(swapchain.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.getExtent();
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (const auto& index_buffer : indexBuffers)
    {
        const auto& vertex_buffer = vertexBuffers[index_buffer.vertexBufferIdx];
        std::vector<VkBuffer> vertex_buffers = {vertex_buffer.pVertexBuffer->getBuffer()};
        std::vector<VkDeviceSize> offsets = {0};
        uint32_t num_bindings = 1;
        const bool use_instancing = (vertex_buffer.pInstanceVertexBuffer != nullptr);
        if (use_instancing)
        {
            vertex_buffers.push_back(vertex_buffer.pInstanceVertexBuffer->getBuffer());
            offsets.push_back(0);
            ++num_bindings;
        }
        vkCmdBindVertexBuffers(command_buffer, 0, num_bindings, vertex_buffers.data(), offsets.data());

        vkCmdBindIndexBuffer(command_buffer, index_buffer.pBuffer->getBuffer(), 0, index_buffer.type);

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &descriptorSets[currentFrame],
            0,
            nullptr);

        vkCmdDrawIndexed(
            command_buffer,
            static_cast<uint32_t>(index_buffer.count),
            static_cast<uint32_t>(vertex_buffer.instanceCount),
            0,
            0,
            0);
    }

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

Renderer::Renderer(Window& window) : window(window), device(window), swapchain(device)
{
    createCommandBuffers();
    createSyncObjects();
}

Renderer::~Renderer()
{
    vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device.getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device.getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device.getLogicalDevice(), inFlightFences[i], nullptr);
    }
}

unsigned Renderer::addUniformBuffer(
    const uint32_t binding,
    const size_t num_bytes,
    const VkShaderStageFlagBits stage_flags,
    const uint32_t array_size)
{
    // 1. Create descriptor set layout binding.
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = binding;
    layout_binding.descriptorCount = array_size;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.stageFlags = stage_flags;
    descriptorSetLayoutBindings.push_back(layout_binding);

    // 2. Create buffer.
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = num_bytes;
    create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    std::vector<std::unique_ptr<Buffer>> bufferPtrsPerFrame(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        bufferPtrsPerFrame[i] = std::make_unique<Buffer>(
            device,
            create_info,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        bufferPtrsPerFrame[i]->map();
    }
    uniformBuffers.emplace_back(binding, std::move(bufferPtrsPerFrame));

    return static_cast<unsigned>(uniformBuffers.size() - 1);
}

void Renderer::updateUniformBuffer(const unsigned index, const void* data, const size_t num_bytes)
{
    uniformBuffers[index].bufferPtrPerFrame[currentFrame]->write(data, num_bytes);
}

void Renderer::addCombinedImageSampler(
    const uint32_t binding,
    const Texture* texture,
    const VkShaderStageFlagBits stage_flags,
    const uint32_t array_size,
    const VkSampler* immutable_samplers)
{
    // 1. Create descriptor set layout binding.
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = binding;
    layout_binding.descriptorCount = array_size;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding.stageFlags = stage_flags;
    layout_binding.pImmutableSamplers = immutable_samplers;
    descriptorSetLayoutBindings.push_back(layout_binding);

    // 2. Save texture for creating descriptor sets.
    combinedImageSamplers.push_back({binding, texture});
}

void Renderer::drawFrame()
{
    // Common outline of rendering a frame:
    //     1. Wait for the previous frame to finish
    //     2. Acquire an image from the swap chain
    //     3. Record a command buffer which draws the scene onto that image
    //     4. Submit the recorded command buffer
    //     5. Present the swap chain image

    // 1.
    vkWaitForFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // 2.
    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        device.getLogicalDevice(),
        swapchain.getSwapchain(),
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        swapchain.recreate();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset the fence if we are submitting work.
    vkResetFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

    // 3.
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], image_index);

    // 4.
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signal_semaphores[] = {renderFinishedSemaphores[currentFrame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submit_info, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // 5.
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {swapchain.getSwapchain()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    present_info.pResults = nullptr; // Optional: good for more than 1 swap chain.

    result = vkQueuePresentKHR(device.getPresentQueue(), &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUBOPTIMAL_KHR || window.resized)
    {
        window.resized = false;
        swapchain.recreate();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
