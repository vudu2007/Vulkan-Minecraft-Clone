#ifndef VMC_SRC_ENGINE_RENDERER_PIPELINE_HPP
#define VMC_SRC_ENGINE_RENDERER_PIPELINE_HPP

#include "device.hpp"

class GraphicsPipeline
{
  private:
    const Device& device;

    VkPipeline pipeline;

  public:
    GraphicsPipeline(const Device& device, const VkGraphicsPipelineCreateInfo& create_info);
    GraphicsPipeline(const GraphicsPipeline& other) = delete;
    GraphicsPipeline(GraphicsPipeline&& other) = delete;
    ~GraphicsPipeline();

    GraphicsPipeline& operator=(const GraphicsPipeline& other) = delete;
    GraphicsPipeline& operator=(GraphicsPipeline&& other) = delete;

    const VkPipeline getPipeline() const;
};

#endif // VMC_SRC_ENGINE_RENDERER_PIPELINE_HPP
