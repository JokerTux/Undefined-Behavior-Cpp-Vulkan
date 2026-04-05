#include <iostream>
#include "surface.h"


bool Init_surface::surface_checks(VkContext& vkcontext){
	uint32_t formatCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext.gpu, vkcontext.surface, &formatCount, 0));
	
	if(formatCount == 0){
		std::cerr << "Surface formats are empty !! " << std::endl;
		return false;
	}
	std::vector<VkSurfaceFormatKHR> in_surfaceFormat;
	in_surfaceFormat.resize(formatCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext.gpu, vkcontext.surface, &formatCount, in_surfaceFormat.data()));

	//fallback if desired format && colorspace not found
	vkcontext.surfaceFormat = in_surfaceFormat[0];	

	for(auto format: in_surfaceFormat){		
		if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
			vkcontext.surfaceFormat = format;
			std::cout << "VK_FORMAT_B8G8R8A8_SRGB + VK_COLOR_SPACE_SRGB_NONLINEAR_KHR : has been found" << std::endl;
			break;
		}

	}

	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkcontext.gpu, vkcontext.surface, &vkcontext.SurfaceCapabilities));

	return true;
}

bool Init_surface::get_ext(VkContext& vkcontext, SDL_Window* window){
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

bool Init_surface::create_sur(VkContext& vkcontext, SDL_Window* window){
	des_context = &vkcontext;
	
	if(!SDL_Vulkan_CreateSurface(window, vkcontext.instance, &vkcontext.surface)){
		std::cout << "surface : " << vkcontext.surface << std::endl;
		std::cerr << "SDL_Vulkan_CreateSurface failed " << std::endl;
		return false; 
	}
	
	return true;
}

Init_surface::~Init_surface(){
	if(des_context && des_context->surface != VK_NULL_HANDLE){
		vkDestroySurfaceKHR(des_context->instance, des_context->surface, nullptr);
		des_context->surface = VK_NULL_HANDLE;
		std::cout << "vkDestroySurfaceKHR has been destroyed" << std::endl;
	}
}