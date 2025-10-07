#include "device.hpp"

#include <cstring>
#include <iostream>

void Device::createAllocator(const VkInstance instance)
{
    // https://stackoverflow.com/questions/73512602/using-vulkan-memory-allocator-with-volk
    VmaVulkanFunctions vma_vulkan_func{};
    vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
    vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
    vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
    vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
    vma_vulkan_func.vkCreateImage = vkCreateImage;
    vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
    vma_vulkan_func.vkDestroyImage = vkDestroyImage;
    vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vma_vulkan_func.vkFreeMemory = vkFreeMemory;
    vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vma_vulkan_func.vkMapMemory = vkMapMemory;
    vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
    vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;

    VmaAllocatorCreateInfo create_info{};
    create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    create_info.vulkanApiVersion = VK_API_VERSION_1_0;
    create_info.physicalDevice = physicalDevice.getHandle();
    create_info.device = logicalDevice.getHandle();
    create_info.instance = instance;
    create_info.pVulkanFunctions = &vma_vulkan_func;

    if (vmaCreateAllocator(&create_info, &allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create VMA allocator!");
    }
}

void Device::createCommandPool(const VkSurfaceKHR surface)
{
    const QueueFamilyIndices queue_family_indices = getQueueFamilies(surface);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphicsFamily.value();

    if (vkCreateCommandPool(logicalDevice.getHandle(), &pool_info, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
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
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = commandPool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(logicalDevice.getHandle(), &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void Device::endSingleTimeCommands(const VkCommandBuffer command_buffer) const
{
    vkEndCommandBuffer(command_buffer);

    // Execute the command.
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(logicalDevice.getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(logicalDevice.getGraphicsQueue());

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

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        // Undefined to transfer destination.
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        // Transfer destination to shader reading.
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(command_buffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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
    const VkFormatProperties format_properties = physicalDevice.getFormatProperties(image_format);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VkCommandBuffer command_buffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;

    // Transition the mip levels.
    for (uint32_t i = 1; i < mip_levels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {(mip_width > 1 ? mip_width / 2 : 1), (mip_height > 1 ? mip_height / 2 : 1), 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

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
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);

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
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);

    endSingleTimeCommands(command_buffer);
}

const VkFormat Device::findSupportedFormat(
    const std::vector<VkFormat>& candidates,
    const VkImageTiling tiling,
    const VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice.getHandle(), format, &properties);

        if (((tiling == VK_IMAGE_TILING_LINEAR) && ((properties.linearTilingFeatures & features) == features)) ||
            ((tiling == VK_IMAGE_TILING_OPTIMAL) && ((properties.optimalTilingFeatures & features) == features)))
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
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

    throw std::runtime_error("failed to find suitable memory type!");
}

const QueueFamilyIndices Device::getQueueFamilies(const VkSurfaceKHR surface) const
{
    return findQueueFamilies(physicalDevice.getHandle(), surface);
}

const SwapchainSupportDetails Device::getSwapchainSupportDetails(const VkSurfaceKHR surface) const
{
    return querySwapChainSupport(physicalDevice.getHandle(), surface);
}

const VkSampleCountFlagBits Device::getMsaaSamples() const
{
    return physicalDevice.getMsaaSamples();
}

const VkPhysicalDeviceProperties Device::getPhysicalDeviceProperties() const
{
    return physicalDevice.getProperties();
}

const VkPhysicalDevice Device::getPhysicalDevice() const
{
    return physicalDevice.getHandle();
}

const VkDevice Device::getLogicalDevice() const
{
    return logicalDevice.getHandle();
}

const VmaAllocator Device::getAllocator() const
{
    return allocator;
}

const VkCommandPool Device::getCommandPool() const
{
    return commandPool;
}

const VkQueue Device::getGraphicsQueue() const
{
    return logicalDevice.getGraphicsQueue();
}

const VkQueue Device::getPresentQueue() const
{
    return logicalDevice.getPresentQueue();
}
