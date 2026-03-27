#pragma once

#include "../platform/SDL_window.h"
#include "vk_utils.h"

bool get_ext(VkContext& vkcontext, SDL_Window* window);
bool create_sur(VkContext& vkcontext, SDL_Window* window);