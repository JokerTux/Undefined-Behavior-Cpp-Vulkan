#include "vk_init.h"
#include "vk_utils.h"
#include <iostream>


bool Vulkan_init::init(VkContext* vkcontext){	
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vk PNG renderer";
	appInfo.pEngineName = "No engine - engine";

	#ifndef NDEBUG
	const char *layers[1] = {"VK_LAYER_KHRONOS_validation"};
	const uint32_t layer_count = 1;
	#else
	const char **layers = nullptr;
	const uint32_t layer_count = 0;
	#endif

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = (uint32_t)vkcontext->extensions.size();
	instanceInfo.ppEnabledExtensionNames = vkcontext->extensions.data();
	instanceInfo.ppEnabledLayerNames = (layer_count > 0) ? layers : nullptr;

	VK_CHECK(vkCreateInstance(&instanceInfo, 0 , &vkcontext->instance));
	std::cout << "Vulkan Initiated." << std::endl;
	return true;
}

// This function does not own the vkcontext items.
/*Vulkan_init::~Vulkan_init(){
	vkDestroyInstance(clear, nullptr);
	std::cout << "Instance has been destroyed" << std::endl;
}*/