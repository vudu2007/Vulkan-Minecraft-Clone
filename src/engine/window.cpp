#include "window.hpp"

#include <cassert>
#include <stdexcept>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    Window* container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    container->isResized = true;
    container->setWidth(width);
    container->setHeight(height);
    container->runResizeCallbacks();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    container->runKeyCallbacks(key, scancode, action, mods);
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* container = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    container->runMouseButtonCallbacks(button, action, mods);
}

Window::Window(const VkInstance instance, const int width, const int height, const std::string& title)
    : width(width), height(height), title(title), instance(instance)
{
    pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (pWindow == nullptr)
    {
        throw std::runtime_error("Failed to create a window!");
    }

    // Create the Vulkan surface.
    VkResult res = glfwCreateWindowSurface(instance, pWindow, nullptr, &surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create a window surface!");
    }

    glfwSetWindowUserPointer(pWindow, this);
    glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
    glfwSetKeyCallback(pWindow, keyCallback);
    glfwSetMouseButtonCallback(pWindow, mouseButtonCallback);
}

Window::~Window()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}

VkSurfaceKHR Window::getSurface() const
{
    assert(surface != VK_NULL_HANDLE && "Window's surface should be initialized when this window was initialized!");
    return surface;
}

int Window::getWidth() const
{
    return width;
}

void Window::setWidth(const int width)
{
    assert(width >= 0 && "Window width cannot be negative!");
    this->width = width;
}

int Window::getHeight() const
{
    return height;
}

void Window::setHeight(const int height)
{
    assert(height >= 0 && "Window height cannot be negative!");
    this->height = height;
}

std::string Window::getTitle() const
{
    return title;
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

int Window::getMouseButtonState(const int button) const
{
    return glfwGetMouseButton(pWindow, button);
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

SubscriberId Window::addResizeCallback(const std::function<void()>& callback)
{
    return resizeCallbacks.subscribe(callback);
}

SubscriberId Window::addKeyCallback(const std::function<void(int, int, int, int)>& callback)
{
    return keyCallbacks.subscribe(callback);
}

SubscriberId Window::addMouseButtonCallback(const std::function<void(int, int, int)>& callback)
{
    return mouseButtonCallbacks.subscribe(callback);
}

void Window::runResizeCallbacks()
{
    resizeCallbacks.notify();
}

void Window::runKeyCallbacks(const int key, const int scancode, const int action, const int mods)
{
    keyCallbacks.notify(key, scancode, action, mods);
}

void Window::runMouseButtonCallbacks(const int button, const int action, const int mods)
{
    mouseButtonCallbacks.notify(button, action, mods);
}
