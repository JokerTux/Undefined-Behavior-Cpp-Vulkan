#pragma once

#include <vulkan/vulkan.h>
#include "vk_utils.h"

class Swapchain_creation{
	public:
		bool queue_dev(VkContext* vkcontext);
		bool create_swap(VkContext* vkcontext);
		
		/*TODO: ~Vulkan_init();*/

	private:
		VkContext* clear = nullptr;
};