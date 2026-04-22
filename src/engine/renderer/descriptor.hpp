#pragma once

#include "device.hpp"

class DescriptorSetLayout
{
  public:
    DescriptorSetLayout(const Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
    DescriptorSetLayout(const DescriptorSetLayout& other) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& other) = delete;

    ~DescriptorSetLayout();

    DescriptorSetLayout& operator=(const DescriptorSetLayout& other) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& other) = delete;

    const VkDescriptorSetLayout getLayout() const;

  private:
    const Device& device;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
};

class DescriptorPool
{
  public:
    DescriptorPool(const Device& device, const std::vector<VkDescriptorPoolSize>& pool_sizes, const uint32_t max_sets);
    DescriptorPool(const DescriptorPool& other) = delete;
    DescriptorPool(DescriptorPool&& other) = delete;

    ~DescriptorPool();

    DescriptorPool& operator=(const DescriptorPool& other) = delete;
    DescriptorPool& operator=(DescriptorPool&& other) = delete;

    std::vector<VkDescriptorSet> allocateDescriptorSets(const DescriptorSetLayout& layout, size_t num_sets) const;
    void freeDescriptorSets(std::vector<VkDescriptorSet>& sets) const;
    void updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& descriptor_writes) const;

    const VkDescriptorPool getPool() const;

  private:
    const Device& device;

    VkDescriptorPool pool = VK_NULL_HANDLE;
};
