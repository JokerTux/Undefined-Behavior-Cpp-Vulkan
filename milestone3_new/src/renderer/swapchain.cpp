#include <iostream>
#include "swapchain.h"
#include "../platform/SDL_window.h"


VkExtent2D swap_choose_extent(VkContext* vkcontext, SDL_Window* window){
	int w = 0, h = 0;
	
	if(vkcontext->SurfaceCapabilities.currentExtent.width != UINT32_MAX){
		std::cout << "Using the fixed surface extent" << std::endl;
		return vkcontext->SurfaceCapabilities.currentExtent;
	}			
	
	SDL_Vulkan_GetDrawableSize(window, &w, &h);
	
	VkExtent2D extent;
	extent.width = static_cast<uint32_t>(w);
	extent.height = static_cast<uint32_t>(h);

	if(extent.width	< vkcontext->SurfaceCapabilities.minImageExtent.width){
		extent.width = vkcontext->SurfaceCapabilities.minImageExtent.width;
	}

	if(extent.width	> vkcontext->SurfaceCapabilities.maxImageExtent.width){
		extent.width = vkcontext->SurfaceCapabilities.maxImageExtent.width;
	}

	if(extent.height < vkcontext->SurfaceCapabilities.minImageExtent.height){
		extent.height = vkcontext->SurfaceCapabilities.minImageExtent.height;
	}

	if(extent.height > vkcontext->SurfaceCapabilities.maxImageExtent.height){
		extent.height = vkcontext->SurfaceCapabilities.maxImageExtent.height;
	}

	return extent;
}


bool Swapchain_creation::create_swap(VkContext* vkcontext, SDL_Window* window){
	des_context = vkcontext;
	//std::cout << "vkcontext->SurfaceCapabilities.minImageCount : " << vkcontext->SurfaceCapabilities.minImageCount << std::endl;
	uint32_t imgCount = vkcontext->SurfaceCapabilities.minImageCount + 1;
	
	if(vkcontext->SurfaceCapabilities.maxImageCount > 0 && imgCount > vkcontext->SurfaceCapabilities.maxImageCount){
        imgCount = vkcontext->SurfaceCapabilities.maxImageCount;
    }

	VkSwapchainCreateInfoKHR swapInfo = {};
	swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapInfo.surface = vkcontext->surface;

	vkcontext->swapchainExtent = swap_choose_extent(vkcontext, window); 

	swapInfo.minImageCount = imgCount;
	swapInfo.imageFormat = vkcontext->surfaceFormat.format;
	swapInfo.imageColorSpace = vkcontext->surfaceFormat.colorSpace;
	swapInfo.imageExtent = vkcontext->swapchainExtent;
	swapInfo.imageArrayLayers = 1;
	swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	swapInfo.preTransform = vkcontext->SurfaceCapabilities.currentTransform;
	swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //TODO : VK_PRESENT_MODE_MAILBOX_KHR - 

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

void Swapchain_creation::cleanup_swap(VkContext* vkcontext){
	for(VkFramebuffer framebuffer : vkcontext->framebuffers){
		if(framebuffer != VK_NULL_HANDLE){
			vkDestroyFramebuffer(vkcontext->device, framebuffer, nullptr);
		}
	}
	vkcontext->framebuffers.clear();

	if(vkcontext->graphicsPipeline != VK_NULL_HANDLE){
		vkDestroyPipeline(vkcontext->device, vkcontext->graphicsPipeline, nullptr);
		vkcontext->graphicsPipeline = VK_NULL_HANDLE;
	}
	
	if(vkcontext->pipelineLayout != VK_NULL_HANDLE){
		vkDestroyPipelineLayout(vkcontext->device, vkcontext->pipelineLayout, nullptr);
		vkcontext->pipelineLayout = VK_NULL_HANDLE;
	}

	if(vkcontext->render_pass != VK_NULL_HANDLE){
		vkDestroyRenderPass(vkcontext->device, vkcontext->render_pass, nullptr);
		vkcontext->render_pass = VK_NULL_HANDLE;
	}

	for(VkImageView view : vkcontext->swapchainImagesViews){
		if(view != VK_NULL_HANDLE){
			vkDestroyImageView(vkcontext->device, view, nullptr);
		}
	}
	vkcontext->swapchainImagesViews.clear();

	if(vkcontext->swapchain != VK_NULL_HANDLE){
		vkDestroySwapchainKHR(vkcontext->device, vkcontext->swapchain, nullptr);
		vkcontext->swapchain = VK_NULL_HANDLE;
	}
	vkcontext->swapchainImages.clear();
}

Swapchain_creation::~Swapchain_creation(){
	if(des_context && des_context->swapchain != VK_NULL_HANDLE){
		vkDestroySwapchainKHR(des_context->device, des_context->swapchain, nullptr);
		des_context->swapchain = VK_NULL_HANDLE;
	}
}