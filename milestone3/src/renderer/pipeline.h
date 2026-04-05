#pragma once

#include "vk_utils.h"

class Graphics_pipeline{
	public:
		bool create_pipeline(VkContext* vkcontext);
		~Graphics_pipeline();
	
	private:
		VkContext* des_context = VK_NULL_HANDLE;
};