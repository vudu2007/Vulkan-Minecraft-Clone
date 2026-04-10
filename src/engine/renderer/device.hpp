#pragma once

#include "logical_device.hpp"
#include "physical_device.hpp"
#include "vulkan_helpers.hpp"

#include "vk_mem_alloc.h"

#include <vector>

class Device
{
  public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    Device(const Device& other) = delete;
    Device(Device&& other) = delete;

    ~Device();

    Device& operator=(const Device& other) = delete;
    Device& operator=(Device&& other) = delete;

    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(const VkCommandBuffer command_buffer) const;

    void transitionImageLayout(
        VkImage image,
        VkFormat format,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        uint32_t mip_levels) const;
    void generateMipmaps(
        VkImage image,
        VkFormat image_format,
        int32_t tex_width,
        int32_t tex_height,
        uint32_t mip_levels) const;

    VkFormat findSupportedFormat(
        const std::vector<VkFormat>& candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features) const;
    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    QueueFamilyIndices getQueueFamilies(VkSurfaceKHR surface) const;
    SwapchainSupportDetails getSwapchainSupportDetails(VkSurfaceKHR surface) const;
    VkSampleCountFlagBits getMsaaSamples() const;
    VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;

    VkPhysicalDevice getPhysicalDevice() const;
    VkDevice getLogicalDevice() const;
    VmaAllocator getAllocator() const;
    VkCommandPool getCommandPool() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;

  private:
    PhysicalDevice physicalDevice;
    LogicalDevice logicalDevice;
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    void createAllocator(VkInstance instance);
    void createCommandPool(VkSurfaceKHR surface);

    bool hasStencilComponent(VkFormat format) const;
};
