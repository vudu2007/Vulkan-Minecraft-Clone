#pragma once

#include "../window.hpp"
#include "device.hpp"

class Swapchain
{
  public:
    Swapchain(const Window& window, const Device& device);
    Swapchain(const Swapchain& other) = delete;
    Swapchain(Swapchain&& other) = delete;
    ~Swapchain();

    Swapchain& operator=(const Swapchain& other) = delete;
    Swapchain& operator=(Swapchain&& other) = delete;

    void recreate();

    // Transition swapchain image at `image_index` to be render attachment optimal.
    void transitionImageLayoutToAttachment(VkCommandBuffer command_buffer, uint32_t image_index);

    // Transition swapchain image at `image_index` to be presentable.
    void transitionImageLayoutToPresent(VkCommandBuffer command_buffer, uint32_t image_index);

    const VkFormat& getFormatRef() const;
    VkFormat getDepthFormat() const;
    VkSwapchainKHR getSwapchain() const;
    VkExtent2D getExtent() const;
    const std::vector<VkImageView>& getImageViews() const;
    const std::vector<VkImageView>& getColorImageViews() const;
    VkImageView getDepthImageView() const;
    size_t getImageCount() const;

  private:
    const Device& device;
    const Window& window;

    VkSwapchainCreateInfoKHR createInfo{};
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat format;
    VkExtent2D extent{};

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    std::vector<VkImage> colorImages;
    std::vector<VmaAllocation> colorImageAllocations;
    std::vector<VkImageView> colorImageViews;

    VkImage depthImage = VK_NULL_HANDLE;
    VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    void createSwapchain();
    void createImageViews();
    void createColorResources();
    void createDepthResources();

    void destroySwapchain();
};
