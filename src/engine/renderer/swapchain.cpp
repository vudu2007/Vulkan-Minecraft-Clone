#include "swapchain.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <stdexcept>

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto& available_format : available_formats)
    {
        if ((available_format.format == VK_FORMAT_B8G8R8A8_SRGB) &&
            (available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        {
            return available_format;
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto& available_present_mode : available_present_modes)
    {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(const Window& window, const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width = -1, height = -1;
        window.getFrameBufferSize(width, height);

        VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actual_extent.width =
            std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height =
            std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

void Swapchain::createSwapchain()
{
    checkVkResult(
        vkCreateSwapchainKHR(device.getLogicalDevice(), &createInfo, nullptr, &swapchain),
        "Failed to create swap chain!");

    // Make sure the container for handles is the correct size.
    checkVkResult(vkGetSwapchainImagesKHR(device.getLogicalDevice(), swapchain, &createInfo.minImageCount, nullptr));
    images.resize(createInfo.minImageCount);
    checkVkResult(
        vkGetSwapchainImagesKHR(device.getLogicalDevice(), swapchain, &createInfo.minImageCount, images.data()));
}

void Swapchain::createImageViews()
{
    imageViews.resize(images.size());

    for (size_t i = 0; i < images.size(); ++i)
    {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = images[i];

        // Interpretation.
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;

        // Color channels.
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Purpose and which part of the image should be accessed.
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        // Create the image view.
        if (vkCreateImageView(device.getLogicalDevice(), &view_info, nullptr, &imageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view!");
        }
    }
}

void Swapchain::createColorResources()
{
    const VkImageCreateInfo image_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent{
            .width = extent.width,
            .height = extent.height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = device.getMsaaSamples(),
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    const VmaAllocationCreateInfo alloc_info{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    assert(!images.empty() && "Cannot create color images if there are no swapchain images!");
    colorImages.resize(images.size(), VK_NULL_HANDLE);
    colorImageAllocations.resize(images.size(), VK_NULL_HANDLE);
    colorImageViews.resize(images.size(), VK_NULL_HANDLE);
    for (size_t i = 0; i < colorImages.size(); ++i)
    {
        // Image and allocation.
        checkVkResult(
            vmaCreateImage(
                device.getAllocator(),
                &image_info,
                &alloc_info,
                &colorImages[i],
                &colorImageAllocations[i],
                nullptr),
            "Failed to create color image and allocate memory!");

        // Associated image view.
        VkImageViewCreateInfo view_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = colorImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        checkVkResult(
            vkCreateImageView(device.getLogicalDevice(), &view_info, nullptr, &colorImageViews[i]),
            "Failed to create color image view!");
    }
}

void Swapchain::createDepthResources()
{
    const VkFormat depth_format = getDepthFormat();

    // Image and memory.
    const VkImageCreateInfo image_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format,
        .extent{
            .width = extent.width,
            .height = extent.height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = device.getMsaaSamples(),
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    const VmaAllocationCreateInfo alloc_info{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    checkVkResult(
        vmaCreateImage(device.getAllocator(), &image_info, &alloc_info, &depthImage, &depthImageAllocation, nullptr),
        "Failed to create depth image and allocate memory!");

    // Image view.
    const VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depth_format,
        .components{
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    checkVkResult(
        vkCreateImageView(device.getLogicalDevice(), &view_info, nullptr, &depthImageView),
        "Failed to create depth image view!");
}

void Swapchain::destroySwapchain()
{
    for (size_t i = 0; i < colorImages.size(); ++i)
    {
        vmaDestroyImage(device.getAllocator(), colorImages[i], colorImageAllocations[i]);
        vkDestroyImageView(device.getLogicalDevice(), colorImageViews[i], nullptr);
    }

    vmaDestroyImage(device.getAllocator(), depthImage, depthImageAllocation);
    vkDestroyImageView(device.getLogicalDevice(), depthImageView, nullptr);

    for (auto image_view : imageViews)
    {
        vkDestroyImageView(device.getLogicalDevice(), image_view, nullptr);
    }

    vkDestroySwapchainKHR(device.getLogicalDevice(), swapchain, nullptr);
}

const VkFormat& Swapchain::getFormatRef() const
{
    return format;
}

VkFormat Swapchain::getDepthFormat() const
{
    return device.findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

Swapchain::Swapchain(const Window& window, const Device& device) : window(window), device(device)
{
    const SwapchainSupportDetails swap_chain_support = device.getSwapchainSupportDetails(window.getSurface());

    const VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(swap_chain_support.formats);
    const VkPresentModeKHR present_mode = chooseSwapPresentMode(swap_chain_support.presentModes);
    const VkExtent2D extent = chooseSwapExtent(window, swap_chain_support.capabilities);

    // Request at least 1 more than minimum in case driver is unable to get another image in a timely manner.
    uint32_t image_count =
        std::min(swap_chain_support.capabilities.minImageCount + 1, swap_chain_support.capabilities.maxImageCount);

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = window.getSurface();
    createInfo.minImageCount = image_count;
    createInfo.imageFormat = surface_format.format;
    createInfo.imageColorSpace = surface_format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swap_chain_support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = present_mode;
    createInfo.clipped = VK_TRUE;

    const QueueFamilyIndices indices = device.getQueueFamilies(window.getSurface());
    const std::vector<uint32_t> queue_family_indices = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queue_family_indices.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional.
        createInfo.pQueueFamilyIndices = nullptr; // Optional.
    }

    // Store for later.
    format = surface_format.format;
    this->extent = extent;

    createSwapchain();
    createImageViews();
    createColorResources();
    createDepthResources();
}

Swapchain::~Swapchain()
{
    destroySwapchain();
}

void Swapchain::recreate()
{
    // Handle window minimization; window is paused until back in foreground.
    int width = -1, height = -1;
    window.getFrameBufferSize(width, height);
    while (width == 0 || height == 0)
    {
        window.getFrameBufferSize(width, height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.getLogicalDevice());

    // Update swapchain create information to match window.
    const SwapchainSupportDetails swap_chain_support = device.getSwapchainSupportDetails(window.getSurface());
    createInfo.oldSwapchain = swapchain; // TODO: figure out when to destroy the old swapchain; ignore for now.
    createInfo.imageExtent = chooseSwapExtent(window, swap_chain_support.capabilities);
    extent = createInfo.imageExtent;

    destroySwapchain();

    createSwapchain();
    createImageViews();
    createColorResources();
    createDepthResources();
}

void Swapchain::transitionImageLayoutToAttachment(const VkCommandBuffer command_buffer, const uint32_t image_index)
{
    const std::vector<VkImageMemoryBarrier2> barriers{
        VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image = images[image_index],
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        },
        VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image = colorImages[image_index],
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        },
        VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image = depthImage,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        },
    };
    const VkDependencyInfo barrier_dependency_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size()),
        .pImageMemoryBarriers = barriers.data(),
    };
    vkCmdPipelineBarrier2(command_buffer, &barrier_dependency_info);
}

void Swapchain::transitionImageLayoutToPresent(const VkCommandBuffer command_buffer, const uint32_t image_index)
{
    const std::vector<VkImageMemoryBarrier2> barriers{
        VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = images[image_index],
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        },
    };
    const VkDependencyInfo barrier_dependency_info{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size()),
        .pImageMemoryBarriers = barriers.data()};
    vkCmdPipelineBarrier2(command_buffer, &barrier_dependency_info);
}

VkSwapchainKHR Swapchain::getSwapchain() const
{
    return swapchain;
}

VkExtent2D Swapchain::getExtent() const
{
    return extent;
}

const std::vector<VkImageView>& Swapchain::getImageViews() const
{
    return imageViews;
}

const std::vector<VkImageView>& Swapchain::getColorImageViews() const
{
    return colorImageViews;
}

VkImageView Swapchain::getDepthImageView() const
{
    return depthImageView;
}
