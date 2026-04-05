#include "instance.h"
#include <iostream>


bool Vulkan_init::init(VkContext* vkcontext){	
	context = vkcontext;

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vk PNG renderer";
	appInfo.pEngineName = "No engine - engine";
	appInfo.apiVersion = VK_API_VERSION_1_2;

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
	instanceInfo.enabledExtensionCount = (uint32_t)context->extensions.size();
	instanceInfo.ppEnabledExtensionNames = context->extensions.data();
	instanceInfo.enabledLayerCount = layer_count;
	instanceInfo.ppEnabledLayerNames = (layer_count > 0) ? layers : nullptr;

	VK_CHECK(vkCreateInstance(&instanceInfo, 0 , &context->instance));
	std::cout << "Vulkan Initiated." << std::endl;
	return true;
}

Vulkan_init::~Vulkan_init(){
	if(context && context->instance != VK_NULL_HANDLE){
		vkDestroyInstance(context->instance, nullptr);
		context->instance = VK_NULL_HANDLE;
		std::cout << "Vulkan_init has been destroyed" << std::endl;
	}
}