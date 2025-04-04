#include "window.hpp"

#include <stdexcept>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    container->resized = true;
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
