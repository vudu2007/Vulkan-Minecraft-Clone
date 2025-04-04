#include "pipeline.hpp"

#include <stdexcept>

GraphicsPipeline::GraphicsPipeline(const Device& device, const VkGraphicsPipelineCreateInfo& create_info)
    : device(device)
{
    if (vkCreateGraphicsPipelines(device.getLogicalDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(device.getLogicalDevice(), pipeline, nullptr);
}

const VkPipeline GraphicsPipeline::getPipeline() const
{
    return pipeline;
}
