#pragma once

#include "event_handler.hpp"
#include "renderer/vulkan_instance.hpp"

#include <functional>
#include <string>
#include <vector>

class Window
{
  public:
    bool isResized = false;

    Window(
        VkInstance instance,
        int width = DEFAULT_WIDTH,
        int height = DEFAULT_HEIGHT,
        const std::string& title = DEFAULT_TITLE);
    Window(const Window& other) = delete;
    Window(Window&& other) = delete;

    ~Window();

    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    VkSurfaceKHR getSurface() const;

    int getWidth() const;
    void setWidth(int width);

    int getHeight() const;
    void setHeight(int height);

    std::string getTitle() const;

    void pollEvents() const;
    bool shouldClose() const;

    void getFrameBufferSize(int& width, int& height) const;
    int getKeyboardKey(int key) const;
    int getMouseButtonState(int button) const;
    void getCursorPosition(double& x, double& y) const;

    int getInputMode(int mode) const;
    void setInputMode(int mode, int value);

    SubscriberId addResizeCallback(const std::function<void()>& callback);
    SubscriberId addKeyCallback(const std::function<void(int, int, int, int)>& callback);
    SubscriberId addMouseButtonCallback(const std::function<void(int, int, int)>& callback);

    void runResizeCallbacks();
    void runKeyCallbacks(int key, int scancode, int action, int mods);
    void runMouseButtonCallbacks(int button, int action, int mods);

  private:
    static constexpr int DEFAULT_WIDTH = 800;
    static constexpr int DEFAULT_HEIGHT = 600;
    static inline const std::string DEFAULT_TITLE = "GLFW Window";

    Publisher<void> resizeCallbacks;
    Publisher<void, int, int, int, int> keyCallbacks;
    Publisher<void, int, int, int> mouseButtonCallbacks;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    GLFWwindow* pWindow;

    int width;
    int height;
    std::string title;
};
