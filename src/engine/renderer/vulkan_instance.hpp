#pragma once

#include "config.hpp"

class VulkanInstance
{
  public:
    VulkanInstance();
    VulkanInstance(const VulkanInstance& other) = delete;
    VulkanInstance(VulkanInstance&& other) = delete;

    ~VulkanInstance();

    VulkanInstance& operator=(const VulkanInstance& other) = delete;
    VulkanInstance& operator=(VulkanInstance&& other) = delete;

    VkInstance getHandle() const;

  private:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    void setupDebugMessenger();
};
