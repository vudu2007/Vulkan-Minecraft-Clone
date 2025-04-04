#ifndef VMC_SRC_ENGINE_RENDERER_DESCRIPTOR_HPP
#define VMC_SRC_ENGINE_RENDERER_DESCRIPTOR_HPP

#include "device.hpp"

class DescriptorSetLayout
{
  private:
    const Device& device;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

  public:
    DescriptorSetLayout(const Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    DescriptorSetLayout(const DescriptorSetLayout& other) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& other) = delete;
    ~DescriptorSetLayout();

    DescriptorSetLayout& operator=(const DescriptorSetLayout& other) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& other) = delete;

    const VkDescriptorSetLayout getLayout() const;
};

class DescriptorPool
{
  private:
    const Device& device;

    VkDescriptorPool pool = VK_NULL_HANDLE;

  public:
    DescriptorPool(const Device& device, const std::vector<VkDescriptorPoolSize>& pool_sizes, const uint32_t max_sets);
    DescriptorPool(const DescriptorPool& other) = delete;
    DescriptorPool(DescriptorPool&& other) = delete;
    ~DescriptorPool();

    DescriptorPool& operator=(const DescriptorPool& other) = delete;
    DescriptorPool& operator=(DescriptorPool&& other) = delete;

    std::vector<VkDescriptorSet> allocateDescriptorSets(const DescriptorSetLayout& layout, const size_t num_sets) const;
    void freeDescriptorSets(std::vector<VkDescriptorSet>& sets) const;
    void updateDescriptorSets(std::vector<VkWriteDescriptorSet> descriptor_writes) const;

    const VkDescriptorPool getPool() const;
};

#endif // VMC_SRC_ENGINE_RENDERER_DESCRIPTOR_HPP
