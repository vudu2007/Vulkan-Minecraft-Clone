#include "descriptor.hpp"

#include <stdexcept>

DescriptorSetLayout::DescriptorSetLayout(
    const Device& device,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    : device(device)
{
    const VkDescriptorSetLayoutCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    checkVkResult(
        vkCreateDescriptorSetLayout(device.getLogicalDevice(), &create_info, nullptr, &layout),
        "Failed to create descriptor set layout");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(device.getLogicalDevice(), layout, nullptr);
}

const VkDescriptorSetLayout DescriptorSetLayout::getLayout() const
{
    return layout;
}

DescriptorPool::DescriptorPool(
    const Device& device,
    const std::vector<VkDescriptorPoolSize>& pool_sizes,
    const uint32_t max_sets)
    : device(device)
{
    const VkDescriptorPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = max_sets,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };
    checkVkResult(
        vkCreateDescriptorPool(device.getLogicalDevice(), &create_info, nullptr, &pool),
        "Failed to create descriptor pool!");
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(device.getLogicalDevice(), pool, nullptr);
}

std::vector<VkDescriptorSet> DescriptorPool::allocateDescriptorSets(
    const DescriptorSetLayout& layout,
    const size_t num_sets) const
{
    const std::vector<VkDescriptorSetLayout> layouts(num_sets, layout.getLayout());
    const VkDescriptorSetAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = static_cast<uint32_t>(num_sets),
        .pSetLayouts = layouts.data(),
    };

    std::vector<VkDescriptorSet> descriptor_sets(num_sets);
    checkVkResult(
        vkAllocateDescriptorSets(device.getLogicalDevice(), &alloc_info, descriptor_sets.data()),
        "Failed to allocate descriptor sets!");

    return descriptor_sets;
}

void DescriptorPool::freeDescriptorSets(std::vector<VkDescriptorSet>& sets) const
{
    vkFreeDescriptorSets(device.getLogicalDevice(), pool, static_cast<uint32_t>(sets.size()), sets.data());
    sets.clear(); // Make sure the sets can't be used after freeing.
}

void DescriptorPool::updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& descriptor_writes) const
{
    vkUpdateDescriptorSets(
        device.getLogicalDevice(),
        static_cast<uint32_t>(descriptor_writes.size()),
        descriptor_writes.data(),
        0,
        nullptr);
}

const VkDescriptorPool DescriptorPool::getPool() const
{
    return pool;
}
