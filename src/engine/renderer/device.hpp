#pragma once

#include "vulkan_instance.hpp"

#include "vk_mem_alloc.h"

#include <optional>
#include <vector>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const;
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device
{
  private:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    void pickPhysicalDevice(const VkInstance instance, const VkSurfaceKHR surface);
    void createLogicalDevice(const VkSurfaceKHR surface);
    void createAllocator(const VkInstance instance);
    void createCommandPool(const VkSurfaceKHR surface);

    bool hasStencilComponent(const VkFormat format) const;

  public:
    Device(const VkInstance instance, const VkSurfaceKHR surface);
    Device(const Device& other) = delete;
    Device(Device&& other) = delete;

    ~Device();

    Device& operator=(const Device& other) = delete;
    Device& operator=(Device&& other) = delete;

    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(const VkCommandBuffer command_buffer) const;

    void transitionImageLayout(
        const VkImage image,
        const VkFormat format,
        const VkImageLayout old_layout,
        const VkImageLayout new_layout,
        const uint32_t mip_levels) const;
    void generateMipmaps(
        const VkImage image,
        const VkFormat image_format,
        const int32_t tex_width,
        const int32_t tex_height,
        const uint32_t mip_levels) const;

    const VkFormat findSupportedFormat(
        const std::vector<VkFormat>& candidates,
        const VkImageTiling tiling,
        const VkFormatFeatureFlags features) const;
    uint32_t findMemoryType(const uint32_t type_filter, const VkMemoryPropertyFlags properties) const;

    const QueueFamilyIndices getQueueFamilies(const VkSurfaceKHR surface) const;
    const SwapchainSupportDetails getSwapchainSupportDetails(const VkSurfaceKHR surface) const;
    const VkSampleCountFlagBits getMsaaSamples() const;
    const VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;

    const VkPhysicalDevice getPhysicalDevice() const;
    const VkDevice getLogicalDevice() const;
    const VmaAllocator getAllocator() const;
    const VkCommandPool getCommandPool() const;
    const VkQueue getGraphicsQueue() const;
    const VkQueue getPresentQueue() const;
};
