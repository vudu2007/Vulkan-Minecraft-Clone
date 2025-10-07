#pragma once

#include "vulkan_instance.hpp"

// TODO: expand on queue handling.
class LogicalDevice
{
  public:
    LogicalDevice(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    LogicalDevice(const LogicalDevice& other) = delete;
    LogicalDevice(LogicalDevice&& other) = delete;

    ~LogicalDevice();

    LogicalDevice& operator=(const LogicalDevice& other) = delete;
    LogicalDevice& operator=(LogicalDevice&& other) = delete;

    VkDevice getHandle() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getPresentQueue() const;

  private:
    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
};
