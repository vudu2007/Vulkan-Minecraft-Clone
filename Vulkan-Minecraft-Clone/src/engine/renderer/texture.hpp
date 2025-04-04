#ifndef VMC_SRC_ENGINE_RENDERER_TEXTURE_HPP
#define VMC_SRC_ENGINE_RENDERER_TEXTURE_HPP

#include "device.hpp"

class Texture
{
  private:
    const Device& device;

    uint32_t mipLevels = 0;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    void createImage(const std::string& texture_file_path);
    void createImageView();
    void createSampler();

  public:
    Texture(const Device& device, const std::string& texture_file_path);
    Texture(const Texture& other) = delete;
    Texture(Texture&& other) = delete;
    ~Texture();

    Texture& operator=(const Texture& other) = delete;
    Texture& operator=(Texture&& other) = delete;

    const VkImageView getImageView() const;
    const VkSampler getSampler() const;
};

#endif // VMC_SRC_ENGINE_RENDERER_TEXTURE_HPP
