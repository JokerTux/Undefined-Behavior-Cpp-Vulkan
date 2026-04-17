#pragma once

#include <SDL2/SDL_vulkan.h>
#include "../platform/SDL_window.h"
#include "vk_utils.h"


class Init_surface{
	public:
		bool get_ext(VkContext& vkcontext, SDL_Window* window);
		bool create_sur(VkContext& vkcontext, SDL_Window* window);
		bool surface_checks(VkContext& vkcontext);
		~Init_surface();

	private:
		VkContext* des_context = nullptr;
};