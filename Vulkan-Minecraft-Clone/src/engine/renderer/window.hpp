#ifndef VMC_SRC_ENGINE_RENDERER_WINDOW_HPP
#define VMC_SRC_ENGINE_RENDERER_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class Window
{
  private:
    static const int DEFAULT_WIDTH = 800;
    static const int DEFAULT_HEIGHT = 600;
    static inline const std::string DEFAULT_TITLE = "GLFW Window";

    GLFWwindow* pWindow;

    int width;
    int height;
    std::string title;

  public:
    bool resized = false;

    Window(
        const int width = DEFAULT_WIDTH,
        const int height = DEFAULT_HEIGHT,
        const std::string& title = DEFAULT_TITLE);
    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    ~Window();

    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    VkSurfaceKHR createSurface(VkInstance instance) const;

    void pollEvents() const;
    bool shouldClose() const;
    void getFrameBufferSize(int& width, int& height) const;
};

#endif // VMC_SRC_ENGINE_RENDERER_WINDOW_HPP
