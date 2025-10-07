#include "logical_device.hpp"

#include "vulkan_helpers.hpp"

#include <set>
#include <stdexcept>

LogicalDevice::LogicalDevice(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    // Specify queues to create.
    QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    // Specify device features.
    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;
    device_features.sampleRateShading = VK_FALSE;
    device_features.fillModeNonSolid = VK_TRUE;

    // Create the logical device.
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures = &device_features;
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    create_info.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    if (ENABLE_VALIDATION_LAYERS)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    // Retrieve queue handles.
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    volkLoadDevice(device);
}

LogicalDevice::~LogicalDevice()
{
    vkDestroyDevice(device, nullptr);
}

VkDevice LogicalDevice::getHandle() const
{
    return device;
}

VkQueue LogicalDevice::getGraphicsQueue() const
{
    return graphicsQueue;
}

VkQueue LogicalDevice::getPresentQueue() const
{
    return presentQueue;
}
