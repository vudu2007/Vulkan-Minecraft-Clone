#include "renderer.hpp"

#include "../../utility.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>

// TODO: look into "bindless" descriptors.
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
    const std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pDescriptorSetLayout->getLayout());
    const VkDescriptorSetAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pDescriptorPool->getPool(),
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = layouts.data(),
    };

    descriptorSets = pDescriptorPool->allocateDescriptorSets(*pDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT);

    const size_t num_buffers = uniformBuffers.size();
    std::vector<std::pair<uint32_t, std::vector<VkDescriptorBufferInfo>>> buffer_infos;

    // Uniform buffers.
    for (size_t i = 0; i < uniformBuffers.size(); ++i)
    {
        std::vector<VkDescriptorBufferInfo> buffer_infos_per_frame;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j)
        {
            const VkDescriptorBufferInfo buffer_info_per_frame{
                .buffer = uniformBuffers[i].bufferPtrPerFrame[j]->getBuffer(),
                .range = uniformBuffers[i].bufferPtrPerFrame[j]->getSize(),
            };
            buffer_infos_per_frame.push_back(buffer_info_per_frame);
        }
        buffer_infos.push_back({uniformBuffers[i].binding, buffer_infos_per_frame});
    }

    const size_t num_images = combinedImageSamplers.size();
    std::vector<std::pair<uint32_t, std::vector<VkDescriptorImageInfo>>> image_infos;

    // Combined Image Samplers.
    for (size_t i = 0; i < combinedImageSamplers.size(); ++i)
    {
        std::vector<VkDescriptorImageInfo> image_infos_per_frame{};
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j)
        {
            const VkDescriptorImageInfo image_info{
                .sampler = combinedImageSamplers[i].texture->getSampler(),
                .imageView = combinedImageSamplers[i].texture->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            image_infos_per_frame.push_back(image_info);
        }
        image_infos.push_back({combinedImageSamplers[i].binding, image_infos_per_frame});
    }

    // Update the descriptor sets.
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        std::vector<VkWriteDescriptorSet> descriptor_writes;
        descriptor_writes.reserve(num_images + num_buffers);

        for (size_t j = 0; j < image_infos.size(); ++j)
        {
            const auto& [binding, sampler_info] = image_infos[j];
            descriptor_writes.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &sampler_info[i],
            });
        }

        for (size_t j = 0; j < buffer_infos.size(); ++j)
        {
            const auto& [binding, buffer_info] = buffer_infos[j];
            descriptor_writes.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSets[i],
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &buffer_info[i],
            });
        }

        pDescriptorPool->updateDescriptorSets(descriptor_writes);
    }
}

bool Renderer::addVertexBuffer(
    const unsigned id,
    const void* data,
    const size_t data_type_size,
    const size_t count,
    const size_t capacity,
    const void* instance_data,
    const size_t instance_data_type_size,
    const size_t instance_count,
    const size_t instance_capacity)
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    // Bad if empty data OR the `id` is already in use OR the capacity is less than count.
    if ((count == 0) || (vertexBuffers.contains(id)) || (capacity < count) || (instance_capacity < instance_count))
    {
        return false;
    }

    // Handle per vertex data.
    size_t num_bytes_capacity = data_type_size * capacity;
    size_t num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = num_bytes_capacity,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
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

    vertexBuffers[id].vertexCount = count;
    vertexBuffers[id].pVertexBuffer = std::make_unique<Buffer>(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
    vertexBuffers[id].pVertexBuffer->copyFrom(staging_buffer, num_bytes);

    // Check if there is per instance data to handle.
    if ((instance_data != nullptr) && (instance_count > 0))
    {
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

        vertexBuffers[id].instanceCount = instance_count;
        vertexBuffers[id].pInstanceVertexBuffer = std::make_unique<Buffer>(
            device,
            create_info,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);
        vertexBuffers[id].pInstanceVertexBuffer->copyFrom(instance_staging_buffer, num_bytes);
    }

    return true;
}

bool Renderer::updateVertexBuffer(const unsigned id, const void* data, const size_t data_type_size, const size_t count)
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    // Ignore if `id` doesn't exist.
    if (!vertexBuffers.contains(id))
    {
        return false;
    }

    vertexBuffers[id].vertexCount = count;

    // Do not attempt to update the buffer if there is no data to update.
    if ((count == 0) || (data == nullptr))
    {
        return true;
    }

    const size_t num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{

        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = num_bytes,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
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
    vertexBuffers[id].pVertexBuffer->copyFrom(staging_buffer, num_bytes);

    return true;
}

bool Renderer::updateInstanceVertexBuffer(
    const unsigned id,
    const void* data,
    const size_t data_type_size,
    const size_t count)
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    // Ignore if `id` doesn't exist.
    if ((!vertexBuffers.contains(id)))
    {
        return false;
    }

    // Do not attempt to update the buffer if there is no data to update.
    if ((count == 0) || (data == nullptr))
    {
        return false;
    }

    const size_t num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = num_bytes,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
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
    vertexBuffers[id].instanceCount = count;
    vertexBuffers[id].pInstanceVertexBuffer->copyFrom(instance_staging_buffer, num_bytes);

    return true;
}

// TODO: look into "VK_KHR_maintenance5" extension; can avoid shader modules and instead do direct passing in pipeline.
VkShaderModule Renderer::createShaderModule(const std::vector<char>& bytecode) const
{
    VkShaderModuleCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = bytecode.size(),
        .pCode = reinterpret_cast<const uint32_t*>(bytecode.data()),
    };

    VkShaderModule shader_module;
    checkVkResult(
        vkCreateShaderModule(device.getLogicalDevice(), &create_info, nullptr, &shader_module),
        "Failed to create a shader module!");

    return shader_module;
}

void Renderer::createGraphicsPipeline()
{
    // Load the shaders.
    auto vert_shader_code = VmcUtility::readFile(VmcUtility::getAssetPath("shaders/shader_block_vert.spv").string());
    auto frag_shader_code = VmcUtility::readFile(VmcUtility::getAssetPath("shaders/shader_block_frag.spv").string());
    VkShaderModule vert_shader_module = createShaderModule(vert_shader_code);
    VkShaderModule frag_shader_module = createShaderModule(frag_shader_code);

    // Handle programmable shaders for pipeline.
    // Vertex.
    const VkPipelineShaderStageCreateInfo vert_shader_stage_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader_module,
        .pName = "main",                // Entrypoint.
        .pSpecializationInfo = nullptr, // Optional. Specify shader constants.
    };

    // Fragment.
    const VkPipelineShaderStageCreateInfo frag_shader_stage_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader_module,
        .pName = "main", // Entrypoint.
    };

    const std::vector<VkPipelineShaderStageCreateInfo> shader_stages{
        vert_shader_stage_info,
        frag_shader_stage_info,
    };

    // Handle fixed stages and other things.
    // Dynamic states are states that can be changed in the fixed pipeline at
    // draw time.
    const std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    const VkPipelineDynamicStateCreateInfo dynamic_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data(),
    };

    // Describe vertex data.
    const std::vector<VkVertexInputBindingDescription> binding_descriptions{
        Model::Vertex::getBindingDescription(),
    };
    const auto& attribute_description_vertex = Model::Vertex::getAttributeDescriptions();
    const std::vector<VkVertexInputAttributeDescription> attribute_descriptions(
        attribute_description_vertex.begin(),
        attribute_description_vertex.end());

    const VkPipelineVertexInputStateCreateInfo vertex_input_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size()),
        .pVertexBindingDescriptions = binding_descriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
        .pVertexAttributeDescriptions = attribute_descriptions.data(),
    };

    // Input assembly.
    const VkPipelineInputAssemblyStateCreateInfo input_assembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    // Viewport and scissors.
    // Viewport is the transformation from an image to the framebuffer.
    const VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchain.getExtent().width),
        .height = static_cast<float>(swapchain.getExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    // Scissor is the crop tool but does not resize the viewport image.
    const VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchain.getExtent(),
    };

    const VkPipelineViewportStateCreateInfo viewport_state{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr, // Set only if not dynamic.
        .scissorCount = 1,
        .pScissors = nullptr, // Set only if not dynamic.
    };

    // Rasterizer.
    const VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,        // Clamp fragments beyond planes to planes.
        .rasterizerDiscardEnable = VK_FALSE, // Geometry never passes through rasterizer stage.
        .polygonMode = VK_POLYGON_MODE_FILL, // Detemines how fragments are generated for geometry.
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE, // Alter depth values by bias.
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    // Multisampling.
    const VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = device.getMsaaSamples(),
        .sampleShadingEnable = VK_FALSE, // Enable sample shading feature for the device.
        .minSampleShading = 0.2f,        // Min. fraction for sample shading; closer to one is smoother.
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    // Depth and stencil testing.
    const VkPipelineDepthStencilStateCreateInfo depth_stencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},            // Optional.
        .back = {},             // Optional.
        .minDepthBounds = 0.0f, // Optional.
        .maxDepthBounds = 1.0f, // Optional.
    };

    // Color blending.
    // Attachment is config per attached framebuffer while create info is global.
    const VkPipelineColorBlendAttachmentState color_blend_attachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional.
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional.
        .colorBlendOp = VK_BLEND_OP_ADD,             // Optional.
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional.
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional.
        .alphaBlendOp = VK_BLEND_OP_ADD,             // Optional.
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    const VkPipelineColorBlendStateCreateInfo color_blending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // Optional.
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants{0.0f, 0.0f, 0.0f, 0.0f}, // Optional.
    };

    // Pipeline layout (+ uniforms).
    const VkDescriptorSetLayout descriptor_set_layout = pDescriptorSetLayout->getLayout();
    const VkPipelineLayoutCreateInfo pipeline_layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,                   // Optional.
        .pSetLayouts = &descriptor_set_layout, // Optional.
        .pushConstantRangeCount = 0,           // Optional.
        .pPushConstantRanges = nullptr,        // Optional.
    };

    checkVkResult(
        vkCreatePipelineLayout(device.getLogicalDevice(), &pipeline_layout_info, nullptr, &pipelineLayout),
        "Failed to create pipeline layout!");

    // Dynamic rendering.
    const VkPipelineRenderingCreateInfo rendering_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchain.getFormatRef(),
        .depthAttachmentFormat = swapchain.getDepthFormat(),
    };

    // Create the graphics pipeline.
    const VkGraphicsPipelineCreateInfo pipeline_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_create_info, // Use dynamic rendering.

        // Shader stages.
        .stageCount = 2,
        .pStages = shader_stages.data(),

        // Fixed stages.
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depth_stencil,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,

        // Layout.
        .layout = pipelineLayout,

        // Pipeline derivative.
        // "These values are only used if the `VK_PIPELINE_CREATE_DERIVATIVE_BIT` flag is also specified in the flags
        // field of `VkGraphicsPipelineCreateInfo`."
        .basePipelineHandle = VK_NULL_HANDLE, // Optional.
        .basePipelineIndex = -1,              // Optional.
    };

    pGraphicsPipeline = std::make_unique<GraphicsPipeline>(device, pipeline_info);

    // Clean up.
    vkDestroyShaderModule(device.getLogicalDevice(), frag_shader_module, nullptr);
    vkDestroyShaderModule(device.getLogicalDevice(), vert_shader_module, nullptr);
}

[[nodiscard]] Texture* Renderer::createTexture(const std::string& path) const
{
    return new Texture(device, path);
}

bool Renderer::addIndexBuffer(
    const unsigned vertex_buffer_id,
    const unsigned index_buffer_id,
    const void* data,
    const size_t data_type_size,
    const size_t count,
    const size_t capacity)
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    const bool is_vert_buff_exist = vertexBuffers.contains(vertex_buffer_id);
    const bool is_index_buff_exist = (vertToIndexBuffers.contains(vertex_buffer_id)) &&
                                     (vertToIndexBuffers[vertex_buffer_id].contains(index_buffer_id));
    const bool is_data_not_exist = (count == 0) || (data == nullptr);
    const bool is_capacity_less = capacity < count;
    if (!is_vert_buff_exist || is_index_buff_exist || is_data_not_exist || is_capacity_less)
    {
        return false;
    }

    const size_t num_bytes_capacity = data_type_size * capacity;
    const VkDeviceSize num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = num_bytes_capacity,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
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

    vertToIndexBuffers[vertex_buffer_id].emplace(
        index_buffer_id,
        IndexBufferInfo(
            count,
            std::make_unique<Buffer>(
                device,
                create_info,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT)));
    vertToIndexBuffers[vertex_buffer_id][index_buffer_id].pBuffer->copyFrom(staging_buffer, num_bytes);

    return true;
}

bool Renderer::updateIndexBuffer(
    const unsigned vertex_buffer_id,
    const unsigned index_buffer_id,
    const void* data,
    const size_t data_type_size,
    const size_t count)
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    const bool is_buffers_not_exist = (!vertexBuffers.contains(vertex_buffer_id)) ||
                                      (!vertToIndexBuffers.contains(vertex_buffer_id)) ||
                                      (!vertToIndexBuffers[vertex_buffer_id].contains(index_buffer_id));
    if (is_buffers_not_exist)
    {
        return false;
    }

    auto& index_buffer = vertToIndexBuffers[vertex_buffer_id][index_buffer_id];
    index_buffer.count = count;

    // Do not attempt to update the buffer if there is no data to update.
    if ((count == 0) || (data == nullptr))
    {
        return false;
    }

    const VkDeviceSize num_bytes = data_type_size * count;

    // Make a staging buffer so that the host can write to it.
    VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = num_bytes,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
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
    index_buffer.pBuffer->copyFrom(staging_buffer, num_bytes);

    return true;
}

void Renderer::removeVertexBuffer(const unsigned id)
{
    vkDeviceWaitIdle(device.getLogicalDevice());
    vertexBuffers.erase(id);
    vertToIndexBuffers.erase(id); // Remove all index buffers associated with the given vertex buffer id.
}

void Renderer::removeIndexBuffer(const unsigned vertex_buffer_id, const unsigned index_buffer_id)
{
    vkDeviceWaitIdle(device.getLogicalDevice());
    auto& index_buffers = vertToIndexBuffers[vertex_buffer_id];
    index_buffers.erase(index_buffer_id);
}

void Renderer::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    const VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = device.getCommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
    };

    checkVkResult(
        vkAllocateCommandBuffers(device.getLogicalDevice(), &alloc_info, commandBuffers.data()),
        "Failed to allocate command buffers!");
}

void Renderer::createSyncObjects()
{
    imageAvailableSemaphores.resize(swapchain.getImageCount());
    renderFinishedSemaphores.resize(swapchain.getImageCount());
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    const VkSemaphoreCreateInfo semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    const VkDevice& logical_device = device.getLogicalDevice();
    for (size_t i = 0; i < swapchain.getImageCount(); ++i)
    {
        checkVkResult(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &imageAvailableSemaphores[i]));
        checkVkResult(vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &renderFinishedSemaphores[i]));
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        checkVkResult(vkCreateFence(logical_device, &fence_info, nullptr, &inFlightFences[i]));
    }
}

void Renderer::recordCommandBuffer(VkCommandBuffer command_buffer, const uint32_t image_index)
{
    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,                  // Optional.
        .pInheritanceInfo = nullptr, // Optional.
    };

    checkVkResult(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to begin recording command buffer!");

    // Dynamic rendering.
    // Transition layout to something more optimal for render attachments.
    swapchain.transitionImageLayoutToAttachment(command_buffer, image_index);

    const VkRenderingAttachmentInfo color_attachment_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,

        .imageView = swapchain.getColorImageViews()[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,

        .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
        .resolveImageView = swapchain.getImageViews()[image_index],
        .resolveImageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,

        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{
            .color{0.43f, 0.7f, 0.92f, 1.0f},
        },
    };
    const VkRenderingAttachmentInfo depth_attachment_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain.getDepthImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue =
            {
                .depthStencil = {1.0f, 0},
            },
    };
    const VkRenderingInfo rendering_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = swapchain.getExtent(),
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    vkCmdBeginRendering(command_buffer, &rendering_info);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline->getPipeline());

    const VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapchain.getExtent().width),
        .height = static_cast<float>(swapchain.getExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    const VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchain.getExtent(),
    };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    for (const auto& vert_to_index_buffers_entry : vertToIndexBuffers)
    {
        const unsigned vertex_buffer_id = vert_to_index_buffers_entry.first;
        const auto& index_buffers = vert_to_index_buffers_entry.second;

        for (const auto& index_buffer_entry : index_buffers)
        {
            const auto& index_buffer = index_buffer_entry.second;

            const auto& vertex_buffer = vertexBuffers[vertex_buffer_id];
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
    }

    vkCmdEndRendering(command_buffer);

    // Transition layout to presentation ready.
    swapchain.transitionImageLayoutToPresent(command_buffer, image_index);

    checkVkResult(vkEndCommandBuffer(command_buffer), "Failed to record command buffer!");
}

Renderer::Renderer(const VkInstance instance, Window& window)
    : window(window), device(instance, window.getSurface()), swapchain(window, device)
{
    createCommandBuffers();
    createSyncObjects();
}

Renderer::~Renderer()
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);

    for (size_t i = 0; i < swapchain.getImageCount(); ++i)
    {
        vkDestroySemaphore(device.getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device.getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
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
    const VkDescriptorSetLayoutBinding layout_binding{
        .binding = binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = array_size,
        .stageFlags = static_cast<VkShaderStageFlags>(stage_flags),
    };
    descriptorSetLayoutBindings.push_back(layout_binding);

    // 2. Create buffer.
    const VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = num_bytes,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

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
    vkWaitForFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
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
    const VkDescriptorSetLayoutBinding layout_binding{
        .binding = binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = array_size,
        .stageFlags = static_cast<VkShaderStageFlags>(stage_flags),
        .pImmutableSamplers = immutable_samplers,
    };
    descriptorSetLayoutBindings.push_back(layout_binding);

    // 2. Save texture for creating descriptor sets.
    combinedImageSamplers.push_back({binding, texture});
}

void Renderer::drawFrame()
{
    // 1. Wait for the previous frame to finish.
    vkWaitForFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // 2. Acquire an image from the swap chain.
    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(
        device.getLogicalDevice(),
        swapchain.getSwapchain(),
        UINT64_MAX,
        imageAvailableSemaphores[swapchainImageIndex],
        VK_NULL_HANDLE,
        &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        swapchain.recreate();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    // Only reset the fence if we are submitting work.
    vkResetFences(device.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

    // 3. Record a command buffer which draws the scene onto that image.
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], image_index);

    // 4. Submit the recorded command buffer.
    const std::vector<VkSemaphore> wait_semaphores{imageAvailableSemaphores[swapchainImageIndex]};
    const std::vector<VkPipelineStageFlags> wait_stages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const std::vector<VkSemaphore> signal_semaphores{renderFinishedSemaphores[swapchainImageIndex]};
    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

        .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.data(),
        .pWaitDstStageMask = wait_stages.data(),

        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffers[currentFrame],

        .signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
        .pSignalSemaphores = signal_semaphores.data(),
    };

    checkVkResult(
        vkQueueSubmit(device.getGraphicsQueue(), 1, &submit_info, inFlightFences[currentFrame]),
        "Failed to submit draw command buffer!");

    // 5. Present the swap chain image.
    const std::vector<VkSwapchainKHR> swapchains{swapchain.getSwapchain()};
    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = signal_semaphores.data(),

        .swapchainCount = static_cast<uint32_t>(swapchains.size()),
        .pSwapchains = swapchains.data(),
        .pImageIndices = &image_index,
    };

    present_info.pResults = nullptr; // Optional: good for more than 1 swap chain.

    result = vkQueuePresentKHR(device.getPresentQueue(), &present_info);
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) || window.isResized)
    {
        window.isResized = false;
        swapchain.recreate();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    swapchainImageIndex = (swapchainImageIndex + 1) % static_cast<uint32_t>(swapchain.getImageCount());
}

Renderer::IndexBufferInfo::IndexBufferInfo(size_t count, std::unique_ptr<Buffer> p_buffer, VkIndexType type)
    : count(count), pBuffer(std::move(p_buffer)), type(type)
{}

Renderer::IndexBufferInfo::IndexBufferInfo(const IndexBufferInfo& other)
{
    // TODO.
    std::cout << "\tIndexBufferInfo copy constructor!" << std::endl;
    count = other.count;
    // pBuffer = std::make_unique<Buffer>(*(other.pBuffer));
    type = other.type;
}

Renderer::IndexBufferInfo::IndexBufferInfo(IndexBufferInfo&& other) noexcept
{
    count = other.count;
    pBuffer = std::move(other.pBuffer);
    type = other.type;
}

Renderer::UniformBufferInfo::UniformBufferInfo(
    const uint32_t binding,
    std::vector<std::unique_ptr<Buffer>>&& buffer_ptr_per_frame)
    : binding(binding), bufferPtrPerFrame(std::move(buffer_ptr_per_frame))
{}
