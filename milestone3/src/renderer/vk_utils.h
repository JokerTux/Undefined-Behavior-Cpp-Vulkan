#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>


#define VK_CHECK(result)                                    \
if(result != VK_SUCCESS){                                   \
    std::cerr << "vulkan error : " << result << std::endl;  \
    return false;                                           \
}

struct VkContext{
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice gpu = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    VkSurfaceFormatKHR surfaceFormat = {};
    VkSurfaceCapabilitiesKHR SurfaceCapabilities = {};
    VkExtent2D swapchainExtent = {};

    uint32_t graphicsIndex = UINT32_MAX;
    uint32_t presentIndex = UINT32_MAX;
    uint32_t currentFrame = 0;
    constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 2; 

    std::vector<const char*> extensions;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImagesViews;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphore;
    std::vector<VkSemaphore> renderFinishedSemaphore;
    std::vector<VkFence> inFlightFence;
};
