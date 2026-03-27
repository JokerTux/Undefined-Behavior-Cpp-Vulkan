#pragma once

#include <SDL2/SDL.h>
#include "vk_utils.h"

class Vulkan_init{
	public:
		bool init(VkContext* vkcontext);
		/*TODO: ~Vulkan_init();*/

	private:
		VkContext* clear = nullptr;
};