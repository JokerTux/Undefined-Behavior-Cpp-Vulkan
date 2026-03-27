#include <iostream>
#include "surface.h"
#include <SDL2/SDL_vulkan.h>

bool get_ext(VkContext& vkcontext){
	unsigned p_count = 0;
	
	if(!SDL_Vulkan_GetInstanceExtensions(window, &p_count, nullptr)){
		std::cerr << "Error : SDL_Vulkan_GetInstanceExtensions : " << SDL_GetError() << std::endl;
		return false;
	}
	vkcontext.extensions.resize(p_count);
	if(!SDL_Vulkan_GetInstanceExtensions(window, &p_count, vkcontext.extensions.data())){
		std::cerr << "Error : SDL_Vulkan_GetInstanceExtensions : " << SDL_GetError() << std::endl;
		return false;
	}
	return true;
}

bool create_sur(VkContext& vkcontext){
	if(!SDL_Vulkan_CreateSurface(window, vkcontext.instance, &vkcontext.surface)){
		std::cerr << "SDL_Vulkan_CreateSurface failed " << std::endl;
		return false; 
	}
	
	return true;
}
	