#pragma once

#include "device.hpp"

class Buffer
{
  public:
    Buffer(
        const Device& device,
        const VkBufferCreateInfo& create_info,
        VmaMemoryUsage mem_usage,
        VmaAllocationCreateFlags mem_flags,
        VkDeviceSize mem_offset = 0);
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other)      = delete;

    ~Buffer();

    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(Buffer&& other)      = delete;

    void map(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
    void unmap();

    void write(const void* data, VkDeviceSize size, VkDeviceSize byte_offset = 0);
    void copyFrom(
        const Buffer& src,
        const VkDeviceSize size,
        const VkDeviceSize src_offset = 0,
        const VkDeviceSize offset     = 0);
    void copyToImage(VkImage dst_image, uint32_t width, uint32_t height);

    VkBuffer getBuffer() const;
    size_t getSize() const;

  private:
    const Device& device;

    VkBuffer buffer          = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;

    void* mappedMemory = nullptr;

    size_t size = 0;
};
