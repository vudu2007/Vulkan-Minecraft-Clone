#pragma once

#include "config.hpp"

#include <optional>

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

QueueFamilyIndices findQueueFamilies(const VkSurfaceKHR surface, const VkPhysicalDevice device);
SwapchainSupportDetails querySwapChainSupport(const VkSurfaceKHR surface, const VkPhysicalDevice device);
