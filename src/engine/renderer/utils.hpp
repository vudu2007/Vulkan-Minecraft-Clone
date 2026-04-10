#pragma once

#include "spdlog/spdlog.h"

#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include <stdexcept>
#include <string>

static inline void checkVkResult(VkResult result, const std::string& failure_message = "")
{
    if (result != VK_SUCCESS)
    {
        spdlog::error(
            "Got bad VkResult ({}) meaning \"{}\"! {}",
            static_cast<int>(result),
            string_VkResult(result),
            failure_message);

#ifdef NDEBUG
        std::exit(result);
#else
        throw std::runtime_error(string_VkResult(result)); // For stack trace if supported.
#endif
    }
}
