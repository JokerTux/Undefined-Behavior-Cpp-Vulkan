/*
#############################################################################################################
	TODO :
		 -select the proper GPU type.
#############################################################################################################
*/

#include "device.h"
#include <cstring>
#include <stdexcept>
#include <set>

struct QueueFamInd{
	uint32_t graphic_fam = 0;
	uint32_t present_fam = 0;
	
	bool graphic_fam_found = false;
	bool present_fam_found = false;
	
	bool is_complete() const{
		return graphic_fam_found && present_fam_found;
	}
};

static QueueFamInd find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface){
	QueueFamInd	indices;

	uint32_t pQueueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &pQueueFamilyPropertyCount, nullptr);

	std::vector<VkQueueFamilyProperties> families(pQueueFamilyPropertyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &pQueueFamilyPropertyCount, families.data());

	for(uint32_t i = 0; i < pQueueFamilyPropertyCount; i++){
		if(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			indices.graphic_fam = i;
			indices.graphic_fam_found = true;
			std::cout << "graphic_fam : " << indices.graphic_fam << std::endl;
		}

		VkBool32 present_support = false;
	    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

		if(present_support){
			indices.present_fam = i;
			indices.present_fam_found = true;
			std::cout << "present_fam : " << indices.present_fam << std::endl;
		}

		if(indices.is_complete()){
			break;
		}
	}
	return indices;

}

static bool support_required_extensions(VkPhysicalDevice device){
	uint32_t pPropertyCount = 0;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &pPropertyCount, nullptr));
	
	if(pPropertyCount == 0){
		std::cerr << "pPropertyCount = 0 " << std::endl;
		return false;
	}

	std::vector<VkExtensionProperties> ext_props(pPropertyCount);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &pPropertyCount, ext_props.data()));
			
	//c++ magic : 
	for(const auto& e_p : ext_props){
		if(strcmp(e_p.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0){
			std::cout << "{VK_KHR_SWAPCHAIN_EXTENSION_NAME} has been found : " << e_p.extensionName << std::endl;
			return true;
		}
	}
	return false;
}

static bool swapchain_is_usable(VkPhysicalDevice device, VkSurfaceKHR surface){
	uint32_t format_count = 0;
	uint32_t present_mode_count = 0;

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

	return format_count > 0 && present_mode_count > 0;
}

static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamInd& out_indices){
	out_indices = find_queue_families(device, surface);

	if(!out_indices.is_complete()){
		return false;
	}

	if(!support_required_extensions(device)){
		return false;
	}

	if(!swapchain_is_usable(device, surface)){
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
	QueueFamInd best_indices{};
	int gpu_type = -1;

	for(const auto& device : devices){
	    VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(device, &props);

		QueueFamInd indices{};
		if(!is_device_suitable(device, vkcontext->surface, indices)){
			continue;
		}
		best_indices = indices;



		/*
		TODO:
		typedef enum VkPhysicalDeviceType {
   		VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
    	VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
    	VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
    	VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
    	VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
		} VkPhysicalDeviceType;*/


		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			gpu_type = 2;
			//chosing the gpu
			best_device = device;

			std::cout << "external gpu type." << std::endl;
			break;
		}
	}
	
	if(best_device == VK_NULL_HANDLE){
		std::cerr << "No suitable GPU found." << std::endl;
		return false;
	}

	vkcontext->gpu = best_device;
	vkcontext->graphicsIndex = best_indices.graphic_fam;
	vkcontext->presentIndex = best_indices.present_fam;

	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(vkcontext->gpu, &props);

	std::cout << "Found GPU: " << props.deviceName << std::endl;
	
	return true;	
}

bool Select_device::create_logical_device(VkContext* vkcontext){
	std::set<uint32_t> u_families ={vkcontext->graphicsIndex, vkcontext->presentIndex};
	des_context = vkcontext;
	float priority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	for(uint32_t family : u_families){
		VkDeviceQueueCreateInfo queue_info{};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = family;
		queue_info.queueCount = 1; //"from this queue I just want one queue"
		queue_info.pQueuePriorities = &priority;
		queueInfos.push_back(queue_info);
	}

	VkPhysicalDeviceFeatures device_features{};

	const char* device_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; //VK_KHR_swapchain

	VkDeviceCreateInfo dev_create_info{};
	dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_create_info.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	dev_create_info.pQueueCreateInfos = queueInfos.data();
	dev_create_info.enabledExtensionCount = 1;
	dev_create_info.ppEnabledExtensionNames = device_exts;
	dev_create_info.pEnabledFeatures = &device_features;

	/* enabledLayerCount and ppEnabledLayerNames are legacy and should not be used
	(https://docs.vulkan.org/refpages/latest/refpages/source/VkDeviceCreateInfo.html)*/
	dev_create_info.enabledLayerCount = 0;
	dev_create_info.ppEnabledLayerNames = nullptr;

	VK_CHECK(vkCreateDevice(vkcontext->gpu, &dev_create_info, nullptr, &vkcontext->device));	
	
	vkGetDeviceQueue(vkcontext->device, vkcontext->graphicsIndex, 0, &vkcontext->graphicsQueue);
	vkGetDeviceQueue(vkcontext->device, vkcontext->presentIndex, 0, &vkcontext->presentQueue);
	
	std::cout << "done" << std::endl;

	return true;
}

Select_device::~Select_device(){
	if(des_context && des_context->device != VK_NULL_HANDLE){
		vkDestroyDevice(des_context->device, nullptr);
		des_context->device = VK_NULL_HANDLE;
	}
}