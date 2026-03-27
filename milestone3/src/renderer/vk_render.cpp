#include <vulkan/vulkan.h>
#include <iostream>
#include "vk_render.h"
#include "surface.h"
#include "vk_utils.h"
#include "../platform/SDL_window.h"




bool devices_search(VkContext* vkcontext){
	uint32_t gpuCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(vkcontext->instance, &gpuCount, nullptr));
	VkPhysicalDevice gpus[gpuCount];
	VK_CHECK(vkEnumeratePhysicalDevices(vkcontext->instance, &gpuCount, gpus));

	for(uint32_t i = 0; i < gpuCount; i++){
		VkPhysicalDevice gpu = gpus[i];
		uint32_t QueueFamilyPropertyCount = 0;
		
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &QueueFamilyPropertyCount, nullptr);
		
		VkQueueFamilyProperties queueProps[QueueFamilyPropertyCount];
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &QueueFamilyPropertyCount, queueProps);
		
		for(uint32_t j = 0; j < QueueFamilyPropertyCount; j++){
			if(queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
				VkBool32 surfaceSupp = VK_FALSE;
				VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, vkcontext->surface, &surfaceSupp));
				
				if(surfaceSupp){
					vkcontext->graphicsIndex = j;
					vkcontext->gpu = gpu;
					break;
				}
			}
		}

		if(vkcontext->graphicsIndex < 0) return false;

		/*std::cout << "gpuCount" << gpuCount << std::endl;
		std::cout << "QueueFamilyPropertyCount" << QueueFamilyPropertyCount << std::endl;
		std::cout << "queueProps" << queueProps << std::endl;
		std::cout << "gpu" << gpu << std::endl;*/
	}
	return true;
}