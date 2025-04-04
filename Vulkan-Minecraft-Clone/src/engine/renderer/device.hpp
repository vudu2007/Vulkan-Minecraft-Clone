#ifndef VMC_SRC_ENGINE_RENDERER_DEVICE_HPP
#define VMC_SRC_ENGINE_RENDERER_DEVICE_HPP

#include "window.hpp"

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
    const Window& window;

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    bool hasStencilComponent(const VkFormat format) const;

  public:
    Device(const Window& window);
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

    const QueueFamilyIndices getQueueFamilies() const;
    const SwapchainSupportDetails getSwapchainSupportDetails() const;
    const VkSampleCountFlagBits getMsaaSamples() const;
    const VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;

    const Window& getWindow() const;
    const VkInstance getInstance() const;
    const VkSurfaceKHR getSurface() const;
    const VkPhysicalDevice getPhysicalDevice() const;
    const VkDevice getLogicalDevice() const;
    const VkCommandPool getCommandPool() const;
    const VkQueue getGraphicsQueue() const;
    const VkQueue getPresentQueue() const;
};

#endif // VMC_SRC_ENGINE_RENDERER_DEVICE_HPP
