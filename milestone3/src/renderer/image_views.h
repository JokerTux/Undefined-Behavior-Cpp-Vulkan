#pragma once

#include "vk_utils.h"
#include <vulkan/vulkan.h>

class Image_views{
	public:
		bool create_views(VkContext* vkcontext);
		~Image_views();
	private:
		VkContext* des_context = VK_NULL_HANDLE;
};