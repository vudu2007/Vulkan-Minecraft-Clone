#pragma once

#include "vulkan_instance.hpp"

class PhysicalDevice
{
  public:
    PhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
    PhysicalDevice(const PhysicalDevice& other) = delete;
    PhysicalDevice(PhysicalDevice&& other) = delete;

    ~PhysicalDevice() = default;

    PhysicalDevice& operator=(const PhysicalDevice& other) = delete;
    PhysicalDevice& operator=(PhysicalDevice&& other) = delete;

    VkPhysicalDevice getHandle() const;

    VkSampleCountFlagBits getMsaaSamples() const;
    VkPhysicalDeviceProperties getProperties() const;
    VkFormatProperties getFormatProperties(VkFormat format) const;
    VkPhysicalDeviceMemoryProperties getMemoryProperties() const;

  private:
    VkPhysicalDevice device = VK_NULL_HANDLE;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};
