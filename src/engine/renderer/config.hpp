#pragma once

#include "utils.hpp"

#include <GLFW/glfw3.h>

#include <vector>

#ifdef NDEBUG
static const bool ENABLE_VALIDATION_LAYERS = false;
#else
static const bool ENABLE_VALIDATION_LAYERS = true;
#endif

static const std::vector<const char*> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static const std::vector<const char*> DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
