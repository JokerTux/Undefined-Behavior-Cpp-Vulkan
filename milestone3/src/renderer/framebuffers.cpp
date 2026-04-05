#include "framebuffers.h"
#include "../platform/SDL_window.h"

VkExtent2D Create_framebuffer::choose_extent(VkContext* vkcontext, SDL_Window* window){
	int w = 0, h = 0;
	//std::cout << "vkcontext->SurfaceCapabilities.maxImageExtent.width : "<< vkcontext->SurfaceCapabilities.maxImageExtent.width << std::endl;
	//std::cout << "vkcontext->SurfaceCapabilities.maxImageExtent.height : "<< vkcontext->SurfaceCapabilities.maxImageExtent.height << std::endl;
	
	if(vkcontext->SurfaceCapabilities.currentExtent.width != UINT32_MAX){
		std::cout << "Using the fixed surface extent" << std::endl;
		return vkcontext->SurfaceCapabilities.currentExtent;
	}			
	
	SDL_Vulkan_GetDrawableSize(window, &w, &h);

	//hmmm ?
	std::cout << "caps width : " << w << std::endl;
	std::cout << "caps height : " << h << std::endl;
	
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

bool Create_framebuffer::create_framebuffer_info(VkContext* vkcontext, SDL_Window* window){
	des_context = vkcontext;
	
	vkcontext->framebuffers.resize(vkcontext->swapchainImagesViews.size());	
	vkcontext->swapchainExtent = choose_extent(vkcontext, window);
	
	for(size_t i = 0; i < vkcontext->swapchainImagesViews.size(); i++){
		VkImageView framebufferAttachments[] = {vkcontext->swapchainImagesViews[i]};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vkcontext->render_pass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = framebufferAttachments;
		framebufferInfo.width = vkcontext->swapchainExtent.width;
		framebufferInfo.height = vkcontext->swapchainExtent.height;
		framebufferInfo.layers = 1;
		
		std::cout << "framebuffer extent width: " << framebufferInfo.width << " height : " << framebufferInfo.height << std::endl;
		VK_CHECK(vkCreateFramebuffer(vkcontext->device, &framebufferInfo, nullptr, &vkcontext->framebuffers[i]));
	}

	return true;
}

Create_framebuffer::~Create_framebuffer(){
	for(VkFramebuffer framebuffer : des_context->framebuffers){
		if(framebuffer != VK_NULL_HANDLE){
			vkDestroyFramebuffer(des_context->device, framebuffer, nullptr);
		}
	}
}