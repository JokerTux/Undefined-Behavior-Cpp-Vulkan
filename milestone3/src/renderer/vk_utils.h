#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

#define VK_CHECK(result)                                    \
if(result != VK_SUCCESS){                                   \
    std::cerr << "vulkan error : " << result << std::endl;  \
    __debugbreak();                                         \
    return false;                                           \
}

struct VkContext {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSwapchainKHR swapchain;

    std::vector<const char*> extensions;
    uint32_t graphicsIndex = UINT32_MAX;
};
