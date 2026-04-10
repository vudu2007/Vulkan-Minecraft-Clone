#include "logical_device.hpp"

#include "vulkan_helpers.hpp"

#include <set>
#include <stdexcept>

LogicalDevice::LogicalDevice(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface)
{
    // TODO: Specify queues to create.
    QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
        const VkDeviceQueueCreateInfo queue_create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
        queue_create_infos.push_back(queue_create_info);
    }

    // Specify device features.
    VkPhysicalDeviceFeatures2 vk_1_0_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features{
            .sampleRateShading = VK_FALSE,
            .fillModeNonSolid = VK_TRUE,
            .samplerAnisotropy = VK_TRUE,
        },
    };
    VkPhysicalDeviceVulkan12Features vk_1_2_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &vk_1_0_features,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true,
    };
    VkPhysicalDeviceVulkan13Features vk_1_3_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vk_1_2_features,
        .synchronization2 = true,
        .dynamicRendering = true,
    };

    // Create the logical device.
    const VkDeviceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &vk_1_3_features,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = ENABLE_VALIDATION_LAYERS ? static_cast<uint32_t>(VALIDATION_LAYERS.size()) : 0,
        .ppEnabledLayerNames = ENABLE_VALIDATION_LAYERS ? VALIDATION_LAYERS.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
        .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
    };

    checkVkResult(vkCreateDevice(physical_device, &create_info, nullptr, &device), "Failed to create logical device!");

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
