#ifndef VMC_SRC_ENGINE_RENDERER_BUFFER_HPP
#define VMC_SRC_ENGINE_RENDERER_BUFFER_HPP

#include "device.hpp"

class Buffer
{
  private:
    const Device& device;

    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    void* mappedMemory = nullptr;

    size_t size = 0;

  public:
    Buffer(
        const Device& device,
        const VkBufferCreateInfo& create_info,
        const VkMemoryPropertyFlags mem_properties,
        const VkDeviceSize mem_offset = 0);
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) = delete;
    ~Buffer();

    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(Buffer&& other) = delete;

    void map(const VkDeviceSize offset = 0, const VkDeviceSize size = VK_WHOLE_SIZE, const VkMemoryMapFlags flags = 0);
    void unmap();

    void write(const void* data, const VkDeviceSize size, const VkDeviceSize byte_offset = 0);
    void copyFrom(
        const Buffer& src,
        const VkDeviceSize size,
        const VkDeviceSize src_offset = 0,
        const VkDeviceSize offset = 0);
    void copyToImage(const VkImage dst_image, const uint32_t width, const uint32_t height);

    const VkBuffer getBuffer() const;
    const size_t getSize() const;
};

#endif // VMC_SRC_ENGINE_RENDERER_BUFFER_HPP
