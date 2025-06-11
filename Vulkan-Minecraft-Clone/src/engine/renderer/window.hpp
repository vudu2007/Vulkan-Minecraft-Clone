#ifndef VMC_SRC_ENGINE_RENDERER_WINDOW_HPP
#define VMC_SRC_ENGINE_RENDERER_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <vector>

class Window
{
  public:
    static const int DEFAULT_WIDTH = 800;
    static const int DEFAULT_HEIGHT = 600;
    static inline const std::string DEFAULT_TITLE = "GLFW Window";

  private:
    GLFWwindow* pWindow;

    int width;
    int height;
    std::string title;

  public:
    std::vector<std::function<void()>> resizeCallbacks;
    std::vector<std::function<void(int, int, int, int)>> keyCallbacks;
    std::vector<std::function<void(int, int, int)>> mouseButtonCallbacks;

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
    int getKeyboardKey(const int key) const;
    int getMouseButtonState(const int button) const;
    void getCursorPosition(double& x, double& y) const;

    int getInputMode(const int mode) const;
    void setInputMode(const int mode, const int value);

    void addResizeCallback(const std::function<void()>& callback);
    void clearResizeCallbacks();
    void addKeyCallback(const std::function<void(int, int, int, int)>& callback);
    void clearKeyCallbacks();
    void addMouseButtonCallback(const std::function<void(int, int, int)>& callback);
    void clearMouseButtonCallbacks();
};

#endif // VMC_SRC_ENGINE_RENDERER_WINDOW_HPP
