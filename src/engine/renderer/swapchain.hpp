#pragma once

#include "device.hpp"

class Swapchain
{
  private:
    const Device& device;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    VkFormat format;
    VkExtent2D extent;

    std::vector<VkImageView> imageViews;

    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkImage colorImage = VK_NULL_HANDLE;
    VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
    VkImageView colorImageView = VK_NULL_HANDLE;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> framebuffers;

    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createColorResources();
    void createDepthResources();
    void createFramebuffers();

    void destroySwapchain();

    const VkFormat findDepthFormat();

  public:
    Swapchain(const Device& device);
    Swapchain(const Swapchain& other) = delete;
    Swapchain(Swapchain&& other) = delete;
    ~Swapchain();

    Swapchain& operator=(const Swapchain& other) = delete;
    Swapchain& operator=(Swapchain&& other) = delete;

    void recreate();

    const VkSwapchainKHR getSwapchain() const;
    const VkExtent2D getExtent() const;
    const VkRenderPass getRenderPass() const;
    const std::vector<VkFramebuffer>& getFramebuffers() const;
};
