#include "window.hpp"

#include <stdexcept>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    container->resized = true;
    for (const auto& callback : container->resizeCallbacks)
    {
        callback();
    }
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    for (const auto& callback : container->keyCallbacks)
    {
        callback(key, scancode, action, mods);
    }
}

Window::Window(const int width, const int height, const std::string& title) : width(width), height(height), title(title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (pWindow == nullptr)
    {
        throw std::runtime_error("failed to create a window!");
    }

    glfwSetWindowUserPointer(pWindow, this);
    glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
    glfwSetKeyCallback(pWindow, keyCallback);
}

Window::~Window()
{
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}

VkSurfaceKHR Window::createSurface(VkInstance instance) const
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, pWindow, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create a window surface!");
    }
    return surface;
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(pWindow);
}

void Window::getFrameBufferSize(int& width, int& height) const
{
    glfwGetFramebufferSize(pWindow, &width, &height);
}

int Window::getKeyboardKey(const int key) const
{
    return glfwGetKey(pWindow, key);
}

void Window::getCursorPosition(double& x, double& y) const
{
    glfwGetCursorPos(pWindow, &x, &y);
}

int Window::getInputMode(const int mode) const
{
    return glfwGetInputMode(pWindow, mode);
}

void Window::setInputMode(const int mode, const int value)
{
    glfwSetInputMode(pWindow, mode, value);
}

void Window::addResizeCallback(const std::function<void()>& callback)
{
    resizeCallbacks.push_back(callback);
}

void Window::clearResizeCallbacks()
{
    resizeCallbacks.clear();
}

void Window::addKeyCallback(const std::function<void(int, int, int, int)>& callback)
{
    keyCallbacks.push_back(callback);
}

void Window::clearKeyCallbacks()
{
    keyCallbacks.clear();
}
