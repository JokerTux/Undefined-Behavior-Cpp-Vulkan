#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include "vk_utils.h"

class Swapchain_creation{
	public:
		bool create_swap(VkContext* vkcontext, SDL_Window* window);
		~Swapchain_creation();

	private:
		VkContext* des_context = VK_NULL_HANDLE;
	};