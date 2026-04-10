#include "vulkan_helpers.hpp"

bool QueueFamilyIndices::isComplete() const
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    QueueFamilyIndices indices{};

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    for (size_t i = 0; i < queue_families.size(); ++i)
    {
        // Check for graphics support.
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        // (Optional) Check for present support.
        if (surface != VK_NULL_HANDLE)
        {
            VkBool32 present_support = false;
            checkVkResult(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support));
            if (present_support)
            {
                indices.presentFamily = i;
            }
        }

        if (indices.isComplete())
        {
            // Found the desired queue family.
            break;
        }
    }

    return indices;
}

SwapchainSupportDetails querySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
    SwapchainSupportDetails details{};

    // Capabilities.
    checkVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities));

    // Formats.
    uint32_t format_count = 0;
    checkVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr));
    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    // Presentation modes.
    uint32_t present_mode_count = 0;
    checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr));
    if (present_mode_count != 0)
    {
        details.presentModes.resize(present_mode_count);
        checkVkResult(vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &present_mode_count,
            details.presentModes.data()));
    }

    return details;
}
