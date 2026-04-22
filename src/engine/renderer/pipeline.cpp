#include "pipeline.hpp"

GraphicsPipeline::GraphicsPipeline(const Device& device, const VkGraphicsPipelineCreateInfo& create_info)
    : device(device)
{
    checkVkResult(
        vkCreateGraphicsPipelines(device.getLogicalDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline),
        "Failed to create graphics pipeline!");
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(device.getLogicalDevice(), pipeline, nullptr);
}

const VkPipeline GraphicsPipeline::getPipeline() const
{
    return pipeline;
}
