#include "vk_utils.h"

bool Swapchain_creation::queue_dev(VkContext* vkcontext){
	const float qPriorities = 1.0f;

	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = vkcontext->graphicsIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &qPriorities;

	char* dev_extensions[0] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.ppEnabledExtensionNames = dev_extensions;
	deviceInfo.enabledExtensionCount = sizeof(dev_extensions) / sizeof(dev_extensions[0]);

	/* enabledLayerCount and ppEnabledLayerNames are legacy and should not be used
			(https://docs.vulkan.org/refpages/latest/refpages/source/VkDeviceCreateInfo.html)*/
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;


	VK_CHECK(vkCreateDevice(vkcontext->gpu, &deviceInfo, nullptr, &vkcontext->device));

	//swapchain
	uint32_t formatCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext->gpu, vkcontext->surface, &formatCount, 0));
	VkSurfaceFormatKHR surfaceFormats[formatCount];
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vkcontext->gpu, vkcontext->surface, &formatCount, surfaceFormats));
	
	for(uint32_t i = 0; i < formatCount; i++){
		VkSurfaceFormatKHR format = surfaceFormats[i];
		if(format.format == VK_FORMAT_B8G8R8A8_SRGB){
			vkcontext->surfaceFormat = format;
			break;
		}
	}

	VkSurfaceCapabilitiesKHR surfaceCaps = {};
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkcontext->gpu, vkcontext->surface, &surfaceCaps));

	return true;
}

bool Swapchain_creation::create_swap(VkContext* vkcontext){
	uint32_t imgCount = surfaceCaps.minImageCount + 1;
	imgCount = imgCount > surfaceCaps.maxImageCount ? imgCount - 1 : imgCount;

	VkSwapchainCreateInfoKHR swapInfo = {};
	swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapInfo.surface = vkcontext->surface;
	swapInfo.imageFormat = vkcontext->surfaceFormat.format;
	swapInfo.preTransform = surfaceCaps.currentTransform;
	swapInfo.imageExtent = surfaceCaps.currentExtent;
	swapInfo.minImageCount = imgCount;
	swapInfo.imageArrayLayers = 1;

	VK_CHECK(vkCreateSwapchainKHR(vkcontext->device, &swapInfo, 0, &vkcontext->swapchain));
	
	return true;
}