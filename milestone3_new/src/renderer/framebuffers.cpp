#include "framebuffers.h"
#include "../platform/SDL_window.h"


bool Create_framebuffer::create_framebuffer_info(VkContext* vkcontext, SDL_Window* window){
	des_context = vkcontext;
	
	vkcontext->framebuffers.resize(vkcontext->swapchainImagesViews.size());	

	
	for(size_t i = 0; i < vkcontext->swapchainImagesViews.size(); i++){
		VkImageView framebufferAttachments[] = {vkcontext->swapchainImagesViews[i]};

		VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
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
	if(des_context != VK_NULL_HANDLE){
		for(VkFramebuffer framebuffer : des_context->framebuffers){
			if(framebuffer != VK_NULL_HANDLE){
				vkDestroyFramebuffer(des_context->device, framebuffer, nullptr);
				framebuffer = VK_NULL_HANDLE;
			}
		}
	}
}