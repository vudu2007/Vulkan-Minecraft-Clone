#include "texture.hpp"

#include "buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <sstream>
#include <stdexcept>

// TODO: consider KTX.
void Texture::createImage(const std::string& texture_file_path)
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(texture_file_path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    VkDeviceSize image_size = static_cast<VkDeviceSize>(tex_width) * tex_height * 4; // 4 bytes per pixel.

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    // Staging phase.
    // Make a staging buffer so that the host can write to it.
    const VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = image_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    Buffer staging_buffer(
        device,
        create_info,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    // The host writes to the staging buffer.
    staging_buffer.map();
    staging_buffer.write(pixels, static_cast<size_t>(image_size));
    staging_buffer.unmap(); // Unmap since host no longer needs to edit it.

    stbi_image_free(pixels);

    const VmaAllocationCreateInfo alloc_info{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    // Create texture image.
    const VkImageCreateInfo image_info{

        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent{
            .width = static_cast<uint32_t>(tex_width),
            .height = static_cast<uint32_t>(tex_height),
            .depth = 1,
        },
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    checkVkResult(
        vmaCreateImage(device.getAllocator(), &image_info, &alloc_info, &image, &allocation, nullptr),
        "Failed to create texture image!");

    // Prepare image for copying into.
    device.transitionImageLayout(
        image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        mipLevels);

    // Copy the buffer to image.
    staging_buffer.copyToImage(image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));

    // Prepare image for the shader(s).
    device.generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, mipLevels);
}

void Texture::createImageView()
{
    const VkImageViewCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,

        // Interpretation.
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,

        // Color channels.
        .components{
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
        },

        // Purpose and which part of the image should be accessed.
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    // Create the image view.
    checkVkResult(
        vkCreateImageView(device.getLogicalDevice(), &create_info, nullptr, &imageView),
        "Failed to create texture image view!");
}

void Texture::createSampler()
{
    const VkPhysicalDeviceProperties properties = device.getPhysicalDeviceProperties();

    const VkSamplerCreateInfo sampler_info{

        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(mipLevels),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    checkVkResult(
        vkCreateSampler(device.getLogicalDevice(), &sampler_info, nullptr, &sampler),
        "Failed to create texture sampler!");
}

Texture::Texture(const Device& device, const std::string& texture_file_path) : device(device)
{
    createImage(texture_file_path);
    createImageView();
    createSampler();
}

Texture::~Texture()
{
    vkDeviceWaitIdle(device.getLogicalDevice());

    vkDestroySampler(device.getLogicalDevice(), sampler, nullptr);
    vkDestroyImageView(device.getLogicalDevice(), imageView, nullptr);
    vmaDestroyImage(device.getAllocator(), image, allocation);
}

const VkImageView Texture::getImageView() const
{
    return imageView;
}

const VkSampler Texture::getSampler() const
{
    return sampler;
}
