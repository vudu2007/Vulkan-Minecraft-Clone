#pragma once

#include "event-handler.hpp"

#include <volk.h>

#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <vector>

class Window
{
  public:
    bool isResized = false;

    Window();
    Window(const int width, const int height, const std::string& title);
    Window(const Window& other) = delete;
    Window(Window&& other) = delete;

    ~Window();

    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    VkSurfaceKHR createSurface(VkInstance instance) const;

    int getWidth() const;
    int getHeight() const;
    std::string getTitle() const;

    void pollEvents() const;
    bool shouldClose() const;

    void getFrameBufferSize(int& width, int& height) const;
    int getKeyboardKey(const int key) const;
    int getMouseButtonState(const int button) const;
    void getCursorPosition(double& x, double& y) const;

    int getInputMode(const int mode) const;
    void setInputMode(const int mode, const int value);

    SubscriberId addResizeCallback(const std::function<void()>& callback);
    SubscriberId addKeyCallback(const std::function<void(int, int, int, int)>& callback);
    SubscriberId addMouseButtonCallback(const std::function<void(int, int, int)>& callback);

    void runResizeCallbacks();
    void runKeyCallbacks(const int key, const int scancode, const int action, const int mods);
    void runMouseButtonCallbacks(const int button, const int action, const int mods);

  private:
    static constexpr int DEFAULT_WIDTH = 800;
    static constexpr int DEFAULT_HEIGHT = 600;
    static inline const std::string DEFAULT_TITLE = "GLFW Window";

    Publisher<void> resizeCallbacks;
    Publisher<void, int, int, int, int> keyCallbacks;
    Publisher<void, int, int, int> mouseButtonCallbacks;

    GLFWwindow* pWindow;

    int width;
    int height;
    std::string title;
};
