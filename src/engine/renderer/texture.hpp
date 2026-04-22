#pragma once

#include "device.hpp"

#include <string>

class Texture
{
  public:
    Texture(const Device& device, const std::string& texture_file_path);
    Texture(const Texture& other) = delete;
    Texture(Texture&& other) = delete;
    ~Texture();

    Texture& operator=(const Texture& other) = delete;
    Texture& operator=(Texture&& other) = delete;

    const VkImageView getImageView() const;
    const VkSampler getSampler() const;

  private:
    const Device& device;

    uint32_t mipLevels = 0;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    void createImage(const std::string& texture_file_path);
    void createImageView();
    void createSampler();
};
