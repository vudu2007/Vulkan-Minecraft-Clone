#include "device.hpp"

#include <cstring>
#include <iostream>

void Device::createAllocator(const VkInstance instance)
{
    // "https://stackoverflow.com/questions/73512602/using-vulkan-memory-allocator-with-volk".
    const VmaVulkanFunctions vma_vk_funcs{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
    };

    const VmaAllocatorCreateInfo create_info{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physicalDevice.getHandle(),
        .device = logicalDevice.getHandle(),
        .pVulkanFunctions = &vma_vk_funcs,
        .instance = instance,
    };

    checkVkResult(vmaCreateAllocator(&create_info, &allocator), "Failed to create VMA allocator!");
}

void Device::createCommandPool(const VkSurfaceKHR surface)
{
    const QueueFamilyIndices queue_family_indices = getQueueFamilies(surface);

    const VkCommandPoolCreateInfo pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_indices.graphicsFamily.value(),
    };

    checkVkResult(
        vkCreateCommandPool(logicalDevice.getHandle(), &pool_info, nullptr, &commandPool),
        "Failed to create command pool!");
}

bool Device::hasStencilComponent(const VkFormat format) const
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

Device::Device(const VkInstance instance, const VkSurfaceKHR surface)
    : physicalDevice(instance, surface), logicalDevice(physicalDevice.getHandle(), surface)
{
    createAllocator(instance);
    createCommandPool(surface);
}

Device::~Device()
{
    vkDestroyCommandPool(logicalDevice.getHandle(), commandPool, nullptr);
    vmaDestroyAllocator(allocator);
}

VkCommandBuffer Device::beginSingleTimeCommands() const
{
    const VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    checkVkResult(vkAllocateCommandBuffers(logicalDevice.getHandle(), &alloc_info, &command_buffer));

    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    checkVkResult(vkBeginCommandBuffer(command_buffer, &begin_info));

    return command_buffer;
}

void Device::endSingleTimeCommands(const VkCommandBuffer command_buffer) const
{
    checkVkResult(vkEndCommandBuffer(command_buffer));

    // Execute the command.
    const VkCommandBufferSubmitInfo command_buffer_submit_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = command_buffer,
    };
    const VkSubmitInfo2 submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_buffer_submit_info,
    };

    checkVkResult(vkQueueSubmit2(logicalDevice.getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));
    checkVkResult(vkQueueWaitIdle(logicalDevice.getGraphicsQueue())); // TODO: use a fence instead!

    vkFreeCommandBuffers(logicalDevice.getHandle(), commandPool, 1, &command_buffer);
}

void Device::transitionImageLayout(
    const VkImage image,
    const VkFormat format,
    const VkImageLayout old_layout,
    const VkImageLayout new_layout,
    const uint32_t mip_levels) const
{
    VkCommandBuffer command_buffer = beginSingleTimeCommands();

    VkImageMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,

        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        // Undefined to transfer destination.
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    }
    else if (
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        // Transfer destination to shader reading.
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        barrier.dstAccessMask =
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    const VkDependencyInfo dependency_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };
    vkCmdPipelineBarrier2(command_buffer, &dependency_info);

    endSingleTimeCommands(command_buffer);
}

void Device::generateMipmaps(
    const VkImage image,
    const VkFormat image_format,
    const int32_t tex_width,
    const int32_t tex_height,
    const uint32_t mip_levels) const
{
    // Check if image format supports linear blitting.
    const VkFormatProperties2 format_properties = physicalDevice.getFormatProperties(image_format);
    if (!(format_properties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VkCommandBuffer command_buffer = beginSingleTimeCommands();

    VkImageMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    const VkDependencyInfo dependency_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };
    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;

    // Transition the mip levels.
    for (uint32_t i = 1; i < mip_levels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);

        const VkImageBlit blit{
            .srcSubresource{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffsets{
                {0, 0, 0},
                {mip_width, mip_height, 1},
            },
            .dstSubresource{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstOffsets{
                {0, 0, 0},
                {(mip_width > 1 ? mip_width / 2 : 1), (mip_height > 1 ? mip_height / 2 : 1), 1},
            },
        };

        vkCmdBlitImage(
            command_buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);

        if (mip_width > 1)
        {
            mip_width /= 2;
        }
        if (mip_height > 1)
        {
            mip_height /= 2;
        }
    }

    // Transition the last mip level.
    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier2(command_buffer, &dependency_info);

    endSingleTimeCommands(command_buffer);
}

VkFormat Device::findSupportedFormat(
    const std::vector<VkFormat>& candidates,
    const VkImageTiling tiling,
    const VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties2 properties{
            .sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2,
        };
        vkGetPhysicalDeviceFormatProperties2(physicalDevice.getHandle(), format, &properties);

        if (((tiling == VK_IMAGE_TILING_LINEAR) &&
             ((properties.formatProperties.linearTilingFeatures & features) == features)) ||
            ((tiling == VK_IMAGE_TILING_OPTIMAL) &&
             ((properties.formatProperties.optimalTilingFeatures & features) == features)))
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}

uint32_t Device::findMemoryType(const uint32_t type_filter, const VkMemoryPropertyFlags properties) const
{
    const VkPhysicalDeviceMemoryProperties mem_properties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

QueueFamilyIndices Device::getQueueFamilies(const VkSurfaceKHR surface) const
{
    return findQueueFamilies(physicalDevice.getHandle(), surface);
}

SwapchainSupportDetails Device::getSwapchainSupportDetails(const VkSurfaceKHR surface) const
{
    return querySwapChainSupport(physicalDevice.getHandle(), surface);
}

VkSampleCountFlagBits Device::getMsaaSamples() const
{
    return physicalDevice.getMsaaSamples();
}

VkPhysicalDeviceProperties Device::getPhysicalDeviceProperties() const
{
    return physicalDevice.getProperties();
}

VkPhysicalDevice Device::getPhysicalDevice() const
{
    return physicalDevice.getHandle();
}

VkDevice Device::getLogicalDevice() const
{
    return logicalDevice.getHandle();
}

VmaAllocator Device::getAllocator() const
{
    return allocator;
}

VkCommandPool Device::getCommandPool() const
{
    return commandPool;
}

VkQueue Device::getGraphicsQueue() const
{
    return logicalDevice.getGraphicsQueue();
}

VkQueue Device::getPresentQueue() const
{
    return logicalDevice.getPresentQueue();
}
