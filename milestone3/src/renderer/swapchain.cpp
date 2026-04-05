#include <iostream>
#include "swapchain.h"

bool Swapchain_creation::create_swap(VkContext* vkcontext, SDL_Window* window){
	des_context = vkcontext;
	//std::cout << "vkcontext->SurfaceCapabilities.minImageCount : " << vkcontext->SurfaceCapabilities.minImageCount << std::endl;
	uint32_t imgCount = vkcontext->SurfaceCapabilities.minImageCount + 1;
	
	if (vkcontext->SurfaceCapabilities.maxImageCount > 0 && imgCount > vkcontext->SurfaceCapabilities.maxImageCount) {
        imgCount = vkcontext->SurfaceCapabilities.maxImageCount;
    }

	VkSwapchainCreateInfoKHR swapInfo = {};
	swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapInfo.surface = vkcontext->surface;
	swapInfo.minImageCount = imgCount;
	swapInfo.imageFormat = vkcontext->surfaceFormat.format;
	swapInfo.imageColorSpace = vkcontext->surfaceFormat.colorSpace;
	swapInfo.imageExtent = vkcontext->SurfaceCapabilities.currentExtent;
	swapInfo.imageArrayLayers = 1;
	swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapInfo.preTransform = vkcontext->SurfaceCapabilities.currentTransform;
	swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapInfo.clipped = VK_TRUE;
	swapInfo.oldSwapchain = VK_NULL_HANDLE;

	uint32_t queue_fam_indexes[] = {vkcontext->graphicsIndex, vkcontext->presentIndex};

	if(vkcontext->graphicsIndex != vkcontext->presentIndex){
		swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapInfo.queueFamilyIndexCount = 2;
		swapInfo.pQueueFamilyIndices = queue_fam_indexes;
	}
	else{
		swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VK_CHECK(vkCreateSwapchainKHR(vkcontext->device, &swapInfo, 0, &vkcontext->swapchain));
	std::cout << "swapchain is working" << std::endl;
	
	uint32_t get_imgCount = 0;
	VK_CHECK(vkGetSwapchainImagesKHR(vkcontext->device, vkcontext->swapchain, &get_imgCount, nullptr));
	vkcontext->swapchainImages.resize(get_imgCount);
	VK_CHECK(vkGetSwapchainImagesKHR(vkcontext->device, vkcontext->swapchain, &get_imgCount, vkcontext->swapchainImages.data()));

	return true;
}

Swapchain_creation::~Swapchain_creation(){
	if(des_context && des_context->swapchain != VK_NULL_HANDLE){
		vkDestroySwapchainKHR(des_context->device, des_context->swapchain, nullptr);
		des_context->swapchain = VK_NULL_HANDLE;
	}
}