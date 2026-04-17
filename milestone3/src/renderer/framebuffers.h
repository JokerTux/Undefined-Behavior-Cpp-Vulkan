#pragma once

#include "vk_utils.h"
#include <SDL2/SDL.h>

class Create_framebuffer{
	public:
		bool create_framebuffer_info(VkContext* vkcontext, SDL_Window* window);
		~Create_framebuffer();

	private:
		VkContext* des_context = nullptr;
};