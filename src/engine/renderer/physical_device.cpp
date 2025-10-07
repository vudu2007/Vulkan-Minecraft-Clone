#include "physical_device.hpp"

#include "vulkan_helpers.hpp"

#include <set>
#include <stdexcept>
#include <string>

static bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    // TODO: add criteria to determine if a physical device is usable for this
    // program.
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    QueueFamilyIndices indices = findQueueFamilies(device, surface);

    // Evaluate device extensions.
    bool extensions_supported = checkDeviceExtensionSupport(device);

    // Check swap chain support.
    bool swap_chain_adequate = false;
    if (extensions_supported)
    {
        SwapchainSupportDetails swap_chain_support = querySwapChainSupport(device, surface);
        swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.presentModes.empty();
    }

    // Check for supported features.
    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    const bool features_supported = supported_features.samplerAnisotropy && supported_features.fillModeNonSolid;

    return (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && indices.isComplete() &&
           extensions_supported && swap_chain_adequate && features_supported;
}

static VkSampleCountFlagBits getMaxUsuableSampleCount(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(device, &physical_device_properties);

    const VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts &
                                      physical_device_properties.limits.framebufferDepthSampleCounts;

    // Must descend since finding max count.
    const VkSampleCountFlagBits sample_counts[] = {
        VK_SAMPLE_COUNT_64_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_2_BIT,
    };
    for (const VkSampleCountFlagBits sample_count : sample_counts)
    {
        if (counts & sample_count)
        {
            return sample_count;
        }
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

PhysicalDevice::PhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, surface))
        {
            this->device = device;
            msaaSamples = getMaxUsuableSampleCount(this->device);
            break;
        }
    }

    if (device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

VkPhysicalDevice PhysicalDevice::getHandle() const
{
    return device;
}

VkSampleCountFlagBits PhysicalDevice::getMsaaSamples() const
{
    return msaaSamples;
}

VkPhysicalDeviceProperties PhysicalDevice::getProperties() const
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device, &properties);
    return properties;
}

VkFormatProperties PhysicalDevice::getFormatProperties(const VkFormat format) const
{
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(device, format, &format_properties);
    return format_properties;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::getMemoryProperties() const
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(device, &mem_properties);
    return mem_properties;
}
