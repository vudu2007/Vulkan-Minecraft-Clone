#pragma once

#include "config.hpp"

#include <optional>

struct QueueFamilyIndices
{
    // TODO: handle other queue families: compute, transfer, sparse binding, etc.
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

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
