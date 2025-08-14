#include "descriptor.hpp"

#include <stdexcept>

DescriptorSetLayout::DescriptorSetLayout(
    const Device& device,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    : device(device)
{
    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device.getLogicalDevice(), &create_info, nullptr, &layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout");
    }
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
    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    create_info.pPoolSizes = pool_sizes.data();
    create_info.maxSets = max_sets;

    if (vkCreateDescriptorPool(device.getLogicalDevice(), &create_info, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool()
{
    vkDestroyDescriptorPool(device.getLogicalDevice(), pool, nullptr);
}

std::vector<VkDescriptorSet> DescriptorPool::allocateDescriptorSets(
    const DescriptorSetLayout& layout,
    const size_t num_sets) const
{
    std::vector<VkDescriptorSetLayout> layouts(num_sets, layout.getLayout());
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(num_sets);
    alloc_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptor_sets(num_sets);
    if (vkAllocateDescriptorSets(device.getLogicalDevice(), &alloc_info, descriptor_sets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return descriptor_sets;
}

void DescriptorPool::freeDescriptorSets(std::vector<VkDescriptorSet>& sets) const
{
    vkFreeDescriptorSets(device.getLogicalDevice(), pool, static_cast<uint32_t>(sets.size()), sets.data());
    sets.clear(); // Make sure the sets can't be used after freeing.
}

void DescriptorPool::updateDescriptorSets(std::vector<VkWriteDescriptorSet> descriptor_writes) const
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
