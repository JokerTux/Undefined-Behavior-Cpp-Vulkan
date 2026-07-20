#include "image_views.h"


bool Image_views::create_views(VkContext* vkcontext){
	des_context	= vkcontext;
	vkcontext->swapchainImagesViews.resize(vkcontext->swapchainImages.size());

	for(size_t i = 0; i < vkcontext->swapchainImages.size(); i++){
		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = vkcontext->swapchainImages[i];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = vkcontext->surfaceFormat.format;

		view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(vkcontext->device, &view_info, nullptr, &vkcontext->swapchainImagesViews[i]));
	}
	return true;
}

Image_views::~Image_views(){
	if(des_context && des_context->device != VK_NULL_HANDLE){
 		for(VkImageView view : des_context->swapchainImagesViews){
			if(view != VK_NULL_HANDLE){
				vkDestroyImageView(des_context->device, view, nullptr);
			}
		}
	}
}