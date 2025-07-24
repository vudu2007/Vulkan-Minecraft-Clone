/* TODO
"It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory for
every individual buffer. The maximum number of simultaneous memory allocations is limited by the
maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create a
custom allocator that splits up a single allocation among many different objects by using the offset parameters
that we've seen in many functions.

You can either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided by the
GPUOpen initiative. However, for this tutorial it's okay to use a separate allocation for every resource,
because we won't come close to hitting any of these limits for now."

----------
----------

"The previous chapter already mentioned that you should allocate multiple resources like buffers from a single
memory allocation, but in fact you should go a step further. Driver developers recommend that you also store
multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like
vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because it's closer
together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used
during the same render operations, provided that their data is refreshed, of course. This is known as aliasing
and some Vulkan functions have explicit flags to specify that you want to do this."
*/

#include "buffer.hpp"

#include <cassert>
#include <sstream>
#include <stdexcept>

Buffer::Buffer(
    const Device& device,
    const VkBufferCreateInfo& create_info,
    const VmaMemoryUsage mem_usage,
    const VmaAllocationCreateFlagBits mem_flags,
    const VkDeviceSize mem_offset)
    : device(device), size(create_info.size)
{
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = mem_usage;
    alloc_info.flags = mem_flags;

    const VkResult res =
        vmaCreateBuffer(device.getAllocator(), &create_info, &alloc_info, &buffer, &allocation, nullptr);
    if (res != VK_SUCCESS)
    {
        std::stringstream err;
        err << "failed to create buffer! code: " << res;
        throw std::runtime_error(err.str());
    }
}

Buffer::~Buffer()
{
    if (mappedMemory != nullptr)
    {
        unmap();
    }
    vmaDestroyBuffer(device.getAllocator(), buffer, allocation);
}

void Buffer::map(const VkDeviceSize offset, const VkDeviceSize size)
{
    if (vmaMapMemory(device.getAllocator(), allocation, &mappedMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to map memory to buffer!");
    }
}

void Buffer::unmap()
{
    vmaUnmapMemory(device.getAllocator(), allocation);
    mappedMemory = nullptr;
}

void Buffer::write(const void* data, const VkDeviceSize size, const VkDeviceSize byte_offset)
{
    assert(mappedMemory != nullptr);
    char* start = static_cast<char*>(mappedMemory) + byte_offset;
    memcpy(start, data, static_cast<size_t>(size));
}

void Buffer::copyFrom(
    const Buffer& src,
    const VkDeviceSize size,
    const VkDeviceSize src_offset,
    const VkDeviceSize offset)
{
    VkCommandBuffer command_buffer = device.beginSingleTimeCommands();

    VkBufferCopy copy_region{};
    copy_region.srcOffset = src_offset; // Optional
    copy_region.dstOffset = offset;     // Optional
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src.buffer, buffer, 1, &copy_region);

    device.endSingleTimeCommands(command_buffer);
}

void Buffer::copyToImage(const VkImage dst_image, const uint32_t width, const uint32_t height)
{
    VkCommandBuffer command_buffer = device.beginSingleTimeCommands();

    // TODO: generalize region
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer, dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    device.endSingleTimeCommands(command_buffer);
}

const VkBuffer Buffer::getBuffer() const
{
    return buffer;
}

const size_t Buffer::getSize() const
{
    return size;
}
