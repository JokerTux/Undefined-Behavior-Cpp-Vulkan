/*
#############################################################################################################
	TODO :
		 -select the proper GPU type.

	Remeber you
#############################################################################################################
*/
#include "device.h"
#include <cstring>
#include <stdexcept>
#include <set>


bool find_queue_families(VkContext* vkcontext, VkPhysicalDevice device){
	uint32_t queue_c = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_c, nullptr);

	std::cout << "queue_c : " << queue_c << std::endl;
	
	std::vector<VkQueueFamilyProperties2> families(queue_c);
	for(VkQueueFamilyProperties2& family : families){
		family.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
	}

	vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_c, families.data());

	for(uint32_t i = 0; i < queue_c; i++){
		if(families[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT){
			vkcontext->graphicsIndex = i;
			std::cout << "grap index : " << i << std::endl;
		}

		VkBool32 present_support = false;
	   vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkcontext->surface, &present_support);

		if(present_support){
			vkcontext->presentIndex = i;
			std::cout << "present index : " << i << std::endl;
			break;
		}
	
	}

	if(queue_c == 0){
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	return true;

}

static bool support_required_extensions(VkPhysicalDevice device){
	bool swap_ext = false;
	bool memo_2_ext = false;

	uint32_t pPropertyCount = ~0;	
	VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &pPropertyCount, nullptr));
	
	std::vector<VkExtensionProperties> ext_props(pPropertyCount);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &pPropertyCount, ext_props.data()));
	
	for(const VkExtensionProperties& e_p : ext_props){
		if(strcmp(e_p.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0){
			std::cout << "Extension has been found : " << e_p.extensionName << std::endl;
			swap_ext = true;
		}

		if(strcmp(e_p.extensionName, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) == 0){
			std::cout << "Extension has been found : " << e_p.extensionName << std::endl;
			memo_2_ext = true;
		}
		if(swap_ext && memo_2_ext) break;
	}
	
	if(!swap_ext || !memo_2_ext){
		return false;
	}

	return true;
}

static bool swapchain_is_usable(VkContext* vkcontext, VkPhysicalDevice device){
	uint32_t format_count = ~0;
	uint32_t present_mode_count = ~0;

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkcontext->surface, &format_count, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkcontext->surface, &present_mode_count, nullptr);

	if(format_count == ~0 || present_mode_count == ~0){
		std::cerr << "vkGetPhysicalDeviceSurfacePresentModesKHR failed" << std::endl;
		return false;
	}

	vkcontext->presentModeIndex = present_mode_count;

	return true;
}

static bool is_device_suitable(VkContext* vkcontext, VkPhysicalDevice device){
	if(!find_queue_families(vkcontext, device)){
		return false;
	}

	if(!support_required_extensions(device)){
		return false;
	}

	if(!swapchain_is_usable(vkcontext,device)){
		return false;
	}

	return true;
}

bool Select_device::pick_physical_device(VkContext* vkcontext){
	uint32_t device_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(vkcontext->instance, &device_count, nullptr));
		
	if(device_count == 0){
		std::cerr << "No Vulkan compatible gpu found." << std::endl;
		return false;
	}
		
	std::vector<VkPhysicalDevice> devices(device_count);
	VK_CHECK(vkEnumeratePhysicalDevices(vkcontext->instance, &device_count, devices.data()));

	VkPhysicalDevice best_device = VK_NULL_HANDLE;
	int gpu_type = -1;

/*TODO:	typedef enum VkPhysicalDeviceType {
   VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
   VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
   VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
   VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
   VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
} VkPhysicalDeviceType;*/

	for(const VkPhysicalDevice& device : devices){
	    VkPhysicalDeviceProperties2 props{};
	    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

		vkGetPhysicalDeviceProperties2(device, &props);
		std::cout << "Props data : " << props.properties.deviceName << std::endl; 
	
		if(!is_device_suitable(vkcontext, device)){
			throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
		}
		best_device = device;

		if(props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			gpu_type = 2;
			best_device = device;
			std::cout << "external gpu type." << std::endl;
			break; //TODO : shoudl i break here ? on my device it works.
		}
	}
	
	if(best_device == VK_NULL_HANDLE){
		std::cerr << "No suitable GPU found." << std::endl;
		return false;
	}

	vkcontext->gpu = best_device;
	/*std::cout << "graphicsIndex :" << vkcontext->graphicsIndex << std::endl;
	std::cout << "presentIndex :" << vkcontext->presentIndex << std::endl;*/
	
	return true;	
}



bool Select_device::create_logical_device(VkContext* vkcontext){
	des_context = vkcontext;
	float priority = 0.5f; //docu says 0.5f
	std::set<uint32_t> u_families ={vkcontext->graphicsIndex, vkcontext->presentIndex};

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT ext{};
	ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
	ext.extendedDynamicState = VK_TRUE;

	VkPhysicalDeviceVulkan13Features vulkan13{};
	vulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan13.dynamicRendering = VK_TRUE; // not checked yet.
	vulkan13.synchronization2 = VK_TRUE;
	vulkan13.pNext = &ext;

	VkPhysicalDeviceFeatures2 features2{};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext = &vulkan13;

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	for(uint32_t family : u_families){
		VkDeviceQueueCreateInfo queue_info{};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = family;
		queue_info.queueCount = 1; //"from this queue I just want one queue"
		queue_info.pQueuePriorities = &priority;
		queueInfos.push_back(queue_info);
	}

	std::vector<const char*> device_exts = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; //VK_KHR_swapchain

	VkDeviceCreateInfo dev_create_info{};
	dev_create_info.pNext = &features2;
	dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_create_info.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	dev_create_info.pQueueCreateInfos = queueInfos.data();
	dev_create_info.enabledExtensionCount = static_cast<uint32_t>(device_exts.size());
	dev_create_info.ppEnabledExtensionNames = device_exts.data();
	dev_create_info.pEnabledFeatures = nullptr;

	dev_create_info.enabledLayerCount = 0;
	dev_create_info.ppEnabledLayerNames = nullptr;

	VK_CHECK(vkCreateDevice(vkcontext->gpu, &dev_create_info, nullptr, &vkcontext->device));	
	
	vkGetDeviceQueue(vkcontext->device, vkcontext->graphicsIndex, 0, &vkcontext->graphicsQueue);
	vkGetDeviceQueue(vkcontext->device, vkcontext->presentIndex, 0, &vkcontext->presentQueue);
	std::cout << "device_exts : " << device_exts[0] << std::endl;
	//std::cout << "device_exts(sizeof, data) :" << sizeof(device_exts) << device_exts<< std::endl;

	return true;
}

Select_device::~Select_device(){
	if(des_context && des_context->device != VK_NULL_HANDLE){
		vkDeviceWaitIdle(des_context->device);
		vkDestroyDevice(des_context->device, nullptr);
		des_context->device = VK_NULL_HANDLE;
	}
}