#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_vulkan.h>


struct Frame_stats{
	double fps;
	double avg_dt;
	double dt;
};

struct QueueFamInd{
	uint32_t graphic_fam = 0;
	uint32_t present_fam = 0;
	
	bool graphic_fam_found = false;
	bool present_fam_found = false;
	
	bool is_complete() const{
		return graphic_fam_found && present_fam_found;
	}
};


class Display_window{
	public:
		bool run_window = true;
		SDL_Window *get_window() const { return window; }
		
		Display_window(){
			window = SDL_CreateWindow("My first vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, (SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN));
			if(!window){
				std::cerr << "Error : SDL_CreateWindow :" << SDL_GetError() << std::endl; 
			}
		}
		~Display_window(){
			SDL_DestroyWindow(window);
			std::cout << "Window has been destroyed" << std::endl; 
		}
	
	void input_from_usr(){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
		   if(event.type == SDL_QUIT){
	    		run_window = false;
	    		break;   
			}

			if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
				std::cout << "esc \n";
		        run_window = false;
		        break;     
			}

			if(event.type == SDL_WINDOWEVENT){
				if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					window_resized = true;
				}
			}
		
		}
	}
	
	Frame_stats delta_time_fps(){
		static Uint64 last = SDL_GetPerformanceCounter();
        static double acc = 0.0;
        static int frames = 0;
        static double avg_dt = 0.0;
        static double fps = 0.0;
        Uint64 now = SDL_GetPerformanceCounter();
        
        double dt = (double)(now - last) / (double)SDL_GetPerformanceFrequency();
 
        last = now;
        acc += dt;
        frames++;
        
        if(acc >= 1.0){
        	fps = frames / acc;
        	avg_dt = acc / frames;
        	
			acc = 0.0;
			frames = 0;
   		}

        return {fps, avg_dt, dt};
    }

    std::vector<const char*> get_vk_ext() const {
    	unsigned p_count = 0;
	
		if(!SDL_Vulkan_GetInstanceExtensions(window, &p_count, nullptr)){
			std::cerr << "Error : SDL_Vulkan_GetInstanceExtensions : " << SDL_GetError() << std::endl;
			return {};
		}
	
		std::vector<const char*>extensions(p_count);
		if(!SDL_Vulkan_GetInstanceExtensions(window, &p_count, extensions.data())){
			std::cerr << "Error : SDL_Vulkan_GetInstanceExtensions : " << SDL_GetError() << std::endl;
	
			return {};
		}
			
		return extensions;
	}
    
    //this is no longer needed since we use another function from vulkan.
    /*Windows_size get_window_details(){
    	SDL_GetWindowSize(window, &window_width, &window_height);
    	return {window_width, window_height};
    }*/
	
	bool window_resized = false;

	private:
		SDL_Window* window = nullptr;
		int window_width = 1280;
		int window_height = 960;
};


class Vulkan_init{
	public:
		Vulkan_init() = default;

		bool init(const std::vector<const char*> &extensions){
	    	return create_instance(extensions);
		}

		VkInstance get_instance() const {return instance;}
		VkSurfaceKHR get_surface() const {return surface;}
		VkExtent2D get_swapchain_extent() const {return swapchainExtent;}

		bool create_instance(const std::vector<const char*> &extensions){
			//metadata
		    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
		    app_info.pApplicationName = "First vulkan";//metadata
		    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);//metadata
		    app_info.pEngineName = "Test engine";//metadata
		    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);//metadata
		    app_info.apiVersion = VK_API_VERSION_1_2;//usable api version

		    //debug stuff | prints debug messages into the console
		    #ifndef NDEBUG
			const char *layers[1] = {"VK_LAYER_KHRONOS_validation"};
			const uint32_t layer_count = 1;
			#else
			const char **layers = nullptr;
			const uint32_t layer_count = 0;
			#endif
		    
		    VkInstanceCreateInfo i_data{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
		    i_data.pApplicationInfo = &app_info;
		    i_data.enabledExtensionCount = (uint32_t)extensions.size(); 
		    i_data.ppEnabledExtensionNames = extensions.data();
		    i_data.enabledLayerCount = layer_count;
		    i_data.ppEnabledLayerNames = (layer_count > 0) ? layers : nullptr;

		    VkResult r = vkCreateInstance(&i_data, nullptr, &instance);

		   	if(r != VK_SUCCESS){
		    	std::cerr << "vkCreateInstance failed : " << r << '\n';
		    	instance = VK_NULL_HANDLE;
		    	return false;
		    }

		    return true;
		}

		bool surface_vk(SDL_Window* window){
			if(!SDL_Vulkan_CreateSurface(window, instance, &surface)){
				std::cerr << "SDL_Vulkan_CreateSurface error : " << SDL_GetError() << '\n';
				if(surface != VK_NULL_HANDLE){
					vkDestroySurfaceKHR(instance, surface, nullptr);
					surface = VK_NULL_HANDLE;
				}

				return false;
			}
			return true;
		}

		std::vector<VkPhysicalDevice> enumerate_gpus(VkInstance instance){
			uint32_t device_count = 0;

			VkResult gpu_r = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

			if(gpu_r != VK_SUCCESS){
				throw std::runtime_error("Failed to enumerate gpu.");
			}

			if(device_count == 0){
				throw std::runtime_error("No device found that is compatible with Vulkan.");
			}
			
			std::vector<VkPhysicalDevice> devices(device_count);
			VkResult devices_r = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
				
			if(devices_r != VK_SUCCESS){
				throw std::runtime_error("Failed to gather devices handles");
			}

			//Print device names.
			for(const auto& device : devices){
			    VkPhysicalDeviceProperties props;
    			vkGetPhysicalDeviceProperties(device, &props);
				std::cout << "Found GPU: " << props.deviceName << std::endl;
			}

			return devices;
		}

		QueueFamInd find_queue_families(VkPhysicalDevice devices, VkSurfaceKHR surface){
			QueueFamInd	indices;
			uint32_t pQueueFamilyPropertyCount = 0;

			vkGetPhysicalDeviceQueueFamilyProperties(devices, &pQueueFamilyPropertyCount, nullptr);

			std::vector<VkQueueFamilyProperties> families(pQueueFamilyPropertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(devices, &pQueueFamilyPropertyCount, families.data());

			for(uint32_t i = 0; i < pQueueFamilyPropertyCount; i++){

				if(!indices.graphic_fam_found && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)){
					indices.graphic_fam = i;
					indices.graphic_fam_found = true;
					std::cout << "graphic_fam : " << indices.graphic_fam << std::endl;
				}

			    VkBool32 present_support = false;
        		vkGetPhysicalDeviceSurfaceSupportKHR(devices, i, surface, &present_support);

	        	if(present_support && !indices.present_fam_found){
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
		
		bool select_device(){
			auto devices = enumerate_gpus(instance);

			for(const auto& device : devices){
				QueueFamInd indices = find_queue_families(device, surface);
				
				if(!indices.is_complete()){
					continue;
				}

				if(!sup_swapchain_ext(device)){
					continue;
				}
				phys_dev = device;

				export_index_gpu_graphic_q = indices.graphic_fam;
				export_index_gpu_present_q = indices.present_fam;
				
				VkPhysicalDeviceProperties props;
    			vkGetPhysicalDeviceProperties(device, &props);
				
				std::cout << "device sleceted : "<< device << std::endl;
				std::cout << "graphic_fam : "<< export_index_gpu_graphic_q << " present_fam : " << export_index_gpu_present_q << std::endl;
				return true;
			}
			return false;
		}

		bool sup_swapchain_ext(VkPhysicalDevice device){
			uint32_t pPropertyCount = 0;

			VkResult r_swap = vkEnumerateDeviceExtensionProperties(device, nullptr, &pPropertyCount, nullptr);
			if(r_swap != VK_SUCCESS || pPropertyCount == 0) return false;

			std::vector<VkExtensionProperties> ext_props(pPropertyCount);
			r_swap = vkEnumerateDeviceExtensionProperties(device, nullptr, &pPropertyCount, ext_props.data());
			if(r_swap != VK_SUCCESS) return false;

			//c++ magic : 
			for(const auto& e_p : ext_props){
				if(strcmp(e_p.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0){
					std::cout << "found {VK_KHR_SWAPCHAIN_EXTENSION_NAME} : " << e_p.extensionName << std::endl;
					return true;
				}
			}
			return false;
		}	

		bool q_device_info(const std::vector<const char*>& device_exts){
			float priority = 1.0f; //the 1.0'f' is required.

			std::vector<VkDeviceQueueCreateInfo> queueInfos;
			//queueInfos capacity= 2
			queueInfos.reserve(2);

			//[&] lambda || & - capture everything by reference | sypported by vulkan
			auto make_queue_info = [&](uint32_t i_fam){
				VkDeviceQueueCreateInfo dev_q{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
				dev_q.queueFamilyIndex = i_fam;
				dev_q.queueCount = 1; //"from this queue I just want one queue"
				dev_q.pQueuePriorities = &priority;
				return dev_q;
			}; // this ; needs to be here in the end since its self evaluating auto make_queue_info var.(I guess)

			queueInfos.push_back(make_queue_info(export_index_gpu_graphic_q));

			if(export_index_gpu_graphic_q != export_index_gpu_present_q){
				queueInfos.push_back(make_queue_info(export_index_gpu_present_q));
			}

			VkDeviceCreateInfo dev_create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
			dev_create_info.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
			dev_create_info.pQueueCreateInfos = queueInfos.data();
			dev_create_info.enabledExtensionCount = 1;
			dev_create_info.ppEnabledExtensionNames = device_exts.data();
			dev_create_info.pEnabledFeatures = &device_features;

			/* enabledLayerCount and ppEnabledLayerNames are legacy and should not be used
			(https://docs.vulkan.org/refpages/latest/refpages/source/VkDeviceCreateInfo.html)*/
			dev_create_info.enabledLayerCount = 0;
			dev_create_info.ppEnabledLayerNames = nullptr;

			

			VkResult create_dev = vkCreateDevice(phys_dev, &dev_create_info, nullptr, &device);
			if(create_dev != VK_SUCCESS){
				std::cerr << "vkCreateDevice failed : " << create_dev << std::endl;
				return false;
			}		

			return true;
		}

		//Logical device creation; 
		bool create_device(){
			//query gpu support 
			vkGetPhysicalDeviceFeatures(phys_dev, &device_features);
			//here I can check if device_features.samplerAnisotropy == 1

			//enables the swapchain capability 
			std::vector<const char*> device_exts = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			if(!q_device_info(device_exts)) return false;

			return true;
		}

		bool queues_vk(){
			//queueIndex = 0 bcs we only have one element in gpu and present families.
			vkGetDeviceQueue(device, export_index_gpu_graphic_q, 0, &gpu_queue);
			vkGetDeviceQueue(device, export_index_gpu_present_q, 0, &present_queue);
			
			if(gpu_queue == VK_NULL_HANDLE || present_queue == VK_NULL_HANDLE){
				return false;
			}

			std::cout << "queues found" << std::endl;

			return true;
		}

		bool sur_cap_KHR(){
			VkResult sur_KHR = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &pSurfaceCapabilities);
			std::cout << "minImageCount :" << pSurfaceCapabilities.minImageCount << std::endl;
			std::cout << "Max Img Count :" << pSurfaceCapabilities.maxImageCount << std::endl;
			std::cout << "currentExtent.width :" << pSurfaceCapabilities.currentExtent.width << std::endl;
			std::cout << "currentExtent.height :" << pSurfaceCapabilities.currentExtent.height << std::endl;
			std::cout << "currentTransform :" << pSurfaceCapabilities.currentTransform << std::endl;

			VkResult phys_sur_KHR = vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &pSurfaceFormatCount, nullptr);
			pSurfaceFormats.resize(pSurfaceFormatCount);
			if(pSurfaceFormats.empty()) return false;
			phys_sur_KHR = vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &pSurfaceFormatCount, pSurfaceFormats.data());

			VkResult phys_sur_pre_mode_KHR = vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &pPresentModeCount, nullptr);
			pPresentModes.resize(pPresentModeCount);
			phys_sur_pre_mode_KHR = vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &pPresentModeCount, pPresentModes.data());

			if(sur_KHR == VK_SUCCESS && phys_sur_KHR == VK_SUCCESS && phys_sur_pre_mode_KHR == VK_SUCCESS){
				return true;
			}
	
			return false;
		}

		bool des_formats_chk(){
			bool found = false;

			if(pSurfaceFormats.empty()) return false;

			for(uint32_t a2 = 0; a2 < pSurfaceFormatCount; a2++){
				//selecting the required format
				if(pSurfaceFormats[a2].format == VK_FORMAT_B8G8R8A8_SRGB && pSurfaceFormats[a2].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
					chosenFormat = pSurfaceFormats[a2];
					found = true;
					break;
				}
			}

			if(!found){
				chosenFormat = pSurfaceFormats[0];
				found = true;
			}
			return found;
		}	

		bool modes_chk(){
			//mailbox cares abut the newest frame|fifo(first in first out) shows every frame even when late.
			if(!pPresentModeCount || pPresentModes.empty() || pPresentModeCount	!= pPresentModes.size()){
				return false;
			}

			chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;

			for(uint32_t a1 = 0; a1 < pPresentModeCount; a1++){
				/*std::cout << "Value : " << std::endl;
				std::cout << pPresentModes.data()[a1] << std::endl;*/
				
				if(pPresentModes[a1] == VK_PRESENT_MODE_MAILBOX_KHR){
					chosenPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					return true;
				}

			}
			return true;
		}

		uint32_t chose_img_count(){
			uint32_t count_img = pSurfaceCapabilities.minImageCount + 1;

			if(pSurfaceCapabilities.maxImageCount > 0 && count_img > pSurfaceCapabilities.maxImageCount){
				count_img = pSurfaceCapabilities.maxImageCount;
			}

			return count_img;
		}

		VkExtent2D choose_extent(SDL_Window* window){
			int w = 0, h = 0;
			std::cout << "pSurfaceCapabilities.maxImageExtent.width : "<< pSurfaceCapabilities.maxImageExtent.width << std::endl;
			std::cout << "pSurfaceCapabilities.maxImageExtent.height : "<< pSurfaceCapabilities.maxImageExtent.height << std::endl;

			if(pSurfaceCapabilities.currentExtent.width != UINT32_MAX){
				return pSurfaceCapabilities.currentExtent;
			}
			
			SDL_Vulkan_GetDrawableSize(window, &w, &h);
			std::cout << "caps width : " << w << std::endl;
			std::cout << "caps height : " << h << std::endl;

			VkExtent2D extent;
			extent.width = static_cast<uint32_t>(w);
			extent.height = static_cast<uint32_t>(h);

			if(extent.width	< pSurfaceCapabilities.minImageExtent.width){
				extent.width = pSurfaceCapabilities.minImageExtent.width;
			}

			if(extent.width	> pSurfaceCapabilities.maxImageExtent.width){
				extent.width = pSurfaceCapabilities.maxImageExtent.width;
			}

			if(extent.height < pSurfaceCapabilities.minImageExtent.height){
				extent.height = pSurfaceCapabilities.minImageExtent.height;
			}

			if(extent.height > pSurfaceCapabilities.maxImageExtent.height){
				extent.height = pSurfaceCapabilities.maxImageExtent.height;
			}

			return extent;
		}

		bool swap_init(SDL_Window* window){
			uint32_t indices[] = {export_index_gpu_graphic_q, export_index_gpu_present_q};

			swapchainExtent = choose_extent(window);

			VkSwapchainCreateInfoKHR swapInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
			swapInfo.surface = surface;
			swapInfo.minImageCount = chose_img_count();
			swapInfo.imageFormat = chosenFormat.format;
			swapInfo.imageColorSpace = chosenFormat.colorSpace;
			swapInfo.imageExtent = swapchainExtent;
			swapInfo.imageArrayLayers = 1; //2d - no depth 
			swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapInfo.preTransform = pSurfaceCapabilities.currentTransform;
			swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			swapInfo.presentMode = chosenPresentMode;
			swapInfo.clipped = VK_TRUE;

			swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if(export_index_gpu_graphic_q != export_index_gpu_present_q){
				swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapInfo.queueFamilyIndexCount = 2;
				swapInfo.pQueueFamilyIndices = indices;
			}

			if(vkCreateSwapchainKHR(device, &swapInfo, nullptr, &swapchain) == VK_SUCCESS){
				return true;	
			}
			else{
				return false;
			}
			
		}

		bool re_swap_init(SDL_Window* window){
			uint32_t indices[] = {export_index_gpu_graphic_q, export_index_gpu_present_q};

			//wait until gpu is not using the swapchain anymore
			vkDeviceWaitIdle(device);

			VkSwapchainKHR old = swapchain;
			swapchainExtent = choose_extent(window);

			VkSwapchainCreateInfoKHR re_swapInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
			re_swapInfo.surface = surface;
			re_swapInfo.minImageCount = chose_img_count();
			re_swapInfo.imageFormat = chosenFormat.format;
			re_swapInfo.imageColorSpace = chosenFormat.colorSpace;
			re_swapInfo.imageExtent = swapchainExtent;
			re_swapInfo.imageArrayLayers = 1; //2d - no depth 
			re_swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			re_swapInfo.preTransform = pSurfaceCapabilities.currentTransform;
			re_swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			re_swapInfo.presentMode = chosenPresentMode;
			re_swapInfo.clipped = VK_TRUE;
			re_swapInfo.oldSwapchain = old;

			re_swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if(export_index_gpu_graphic_q != export_index_gpu_present_q){
				re_swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				re_swapInfo.queueFamilyIndexCount = 2;
				re_swapInfo.pQueueFamilyIndices = indices;
			}


			VkResult r_re_swap = vkCreateSwapchainKHR(device, &re_swapInfo, nullptr, &swapchain);
			if(r_re_swap != VK_SUCCESS) return false;

			std::cout << "New swapchainExtent width: " << swapchainExtent.width << " height : " << swapchainExtent.height << std::endl;

			if(old != VK_NULL_HANDLE){
				vkDestroySwapchainKHR(device, old, nullptr);
			}

			return true;			
		}

		bool swap_get_images(){
			VkResult get_img_swap = vkGetSwapchainImagesKHR(device, swapchain, &pSwapchainImageCount, nullptr);
			
			std::cout << pSwapchainImageCount << std::endl;
			
			if(get_img_swap == VK_SUCCESS){
				swapchainImages.resize(pSwapchainImageCount);
				VkResult swap_imgs = vkGetSwapchainImagesKHR(device, swapchain, &pSwapchainImageCount, swapchainImages.data());
				/*for(uint32_t a111 = 0; a111 < pSwapchainImageCount; a111++){
					std::cout << swapchainImages.data()[a111] << std::endl;
				}*/
				if(swap_imgs == VK_SUCCESS) return true;
			}
							
			return false;	
		}

		bool create_image_views(){
			swapchainImagesView.resize(swapchainImages.size());

			for(size_t ab12 = 0; ab12 < swapchainImagesView.size(); ab12++){
				VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
				
				view_info.image = swapchainImages[ab12];
				view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				view_info.format = chosenFormat.format;

				view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

				view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				view_info.subresourceRange.baseMipLevel = 0;
				view_info.subresourceRange.levelCount = 1;
				view_info.subresourceRange.baseArrayLayer = 0;
				view_info.subresourceRange.layerCount = 1;

				VkResult r_view_info = vkCreateImageView(device, &view_info, nullptr, &swapchainImagesView[ab12]);
				if(r_view_info != VK_SUCCESS){
					std::cerr << "vkCreateImageView failed at index : " << ab12 << std::endl;
					return false;
				}
			}

			return true;
		}

		bool create_render_pass(){
			//std::cout << "chosenFormat.format : " << chosenFormat.format << std::endl;

			// WTF !!!!
			VkAttachmentDescription color_attachments{};
			color_attachments.format = chosenFormat.format;
			color_attachments.samples = VK_SAMPLE_COUNT_1_BIT;
			color_attachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			color_attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_attachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			color_attachments.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference color_attachment_ref{};
			color_attachment_ref.attachment = 0;
			color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment_ref;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
			render_pass_info.attachmentCount = 1;
			render_pass_info.pAttachments = &color_attachments;
			render_pass_info.subpassCount = 1;
			render_pass_info.pSubpasses = &subpass;
			render_pass_info.dependencyCount = 1;
			render_pass_info.pDependencies = &dependency;

			VkResult r_render_pass = vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass);
			
			if(r_render_pass != VK_SUCCESS){
				std::cerr << "vkCreateRenderPass failed " << std::endl;
				return false;
			}

			return true;
		}

		bool create_framebuffers(){
			swapchainFramebuffers.resize(swapchainImagesView.size());

			for(size_t ab123 = 0; ab123 < swapchainImagesView.size(); ab123++){
				VkImageView framebufferAttachments[] = {swapchainImagesView[ab123]};

			
				VkFramebufferCreateInfo framebufferInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
				framebufferInfo.renderPass = render_pass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = framebufferAttachments;
				framebufferInfo.width = swapchainExtent.width;
				framebufferInfo.height = swapchainExtent.height;
				framebufferInfo.layers = 1;

				std::cout << "framebuffer extent width: " << framebufferInfo.width << " height : " << framebufferInfo.height << std::endl;

				VkResult r_createFramebuffer = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapchainFramebuffers[ab123]);
				if(r_createFramebuffer != VK_SUCCESS){
					std::cerr << "vkCreateFramebuffer failed at index[ab123] : " << ab123 << std::endl;
					return false;
				}
			}
			return true;
		}

		bool create_command_pool(){
			VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = export_index_gpu_graphic_q;

			VkResult r_createCommandPool = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
			if(r_createCommandPool != VK_SUCCESS){
				std::cerr << "VkCreateCommand failed" << std::endl;
				return false;
			}
			return true;
		}

		bool create_command_buffers(){
			commandBuffers.resize(swapchainFramebuffers.size());

			VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

			VkResult r_allocCommandBuffers = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
			if(r_allocCommandBuffers != VK_SUCCESS){
				std::cerr << "vkAllocateCommandBuffers failed " << std::endl;
				return false;
			}
			return true;
		}

		bool record_command_buff(){
			if(swapchainExtent.width == 0 || swapchainExtent.height == 0){
				return true;
			}

			for(size_t ab1234 = 0; ab1234 < commandBuffers.size(); ab1234++){
				VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
				beginInfo.flags = 0;
				beginInfo.pInheritanceInfo = nullptr;

				VkResult r_begin = vkBeginCommandBuffer(commandBuffers[ab1234], &beginInfo);
				if(r_begin != VK_SUCCESS){
					std::cerr << "vkBeginCommandBuffer failed at index : " << ab1234 << std::endl;
					return false;
				}

				VkClearValue clearColor = {{{1.0f, 0.1f, 0.1f, 1.0f}}};

				VkRenderPassBeginInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
				renderPassInfo.renderPass = render_pass;
				renderPassInfo.framebuffer = swapchainFramebuffers[ab1234];
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = swapchainExtent;
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(commandBuffers[ab1234], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdEndRenderPass(commandBuffers[ab1234]);

				VkResult r_end = vkEndCommandBuffer(commandBuffers[ab1234]);
				if(r_end != VK_SUCCESS){
					std::cerr << "vkEndCommandBuffer failed at index : " << ab1234 << std::endl;
					return false;
				}
			}
			return true;
		}

		bool create_sync_objects(){
			imageAvailableSemaphore.resize(max_frames_in_flight);
			renderFinishedSemaphore.resize(max_frames_in_flight);
			inFlightFence.resize(max_frames_in_flight);

			VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
			VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for(size_t i = 0; i < max_frames_in_flight; i++){
				VkResult r1 = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]);
				VkResult r2 = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]);
				VkResult r3 = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence[i]);

				if(r1 != VK_SUCCESS || r2 != VK_SUCCESS || r3 != VK_SUCCESS){
					std::cerr << "failed to create sync objects " << std::endl;
					return false;
				}
			}

			imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

			return true;
		}

		bool draw_frame(SDL_Window* window, bool& window_resized){
			int w = 0, h = 0;

			SDL_Vulkan_GetDrawableSize(window, &w, &h);
			if(w == 0 || h == 0){
				return true;
			}

			vkWaitForFences(device, 1, &inFlightFence[current_frame], VK_TRUE, UINT64_MAX);
			uint32_t imageIndex = 0;

			VkResult r_acquire = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore[current_frame], VK_NULL_HANDLE, &imageIndex);
			
			if(r_acquire == VK_ERROR_OUT_OF_DATE_KHR){
				return re_swap(window);
			}
			else if(r_acquire != VK_SUCCESS && r_acquire != VK_SUBOPTIMAL_KHR){
				std::cerr << "vkAcquireNextImageKHR failed : " << r_acquire << std::endl;
				return false;
			}

			if(imagesInFlight[imageIndex] != VK_NULL_HANDLE){
				vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
			}

			imagesInFlight[imageIndex] = inFlightFence[current_frame];
			vkResetFences(device, 1, &inFlightFence[current_frame]);

			VkSemaphore waitSemaphores[] = {imageAvailableSemaphore[current_frame]};
			VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
			VkSemaphore signalSemaphores[] = {renderFinishedSemaphore[current_frame]};

			VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			VkResult r_submit = vkQueueSubmit(gpu_queue, 1, &submitInfo, inFlightFence[current_frame]);
			if(r_submit != VK_SUCCESS){
				std::cerr << "vkQueueSubmit failed : " << r_submit << std::endl;
				return false;
			}

			VkSwapchainKHR swapchains[] = {swapchain};

			VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapchains;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.pResults = nullptr;

			VkResult r_present = vkQueuePresentKHR(present_queue, &presentInfo);
			if(r_present == VK_ERROR_OUT_OF_DATE_KHR || r_present == VK_SUBOPTIMAL_KHR || window_resized){
				window_resized = false;
				return re_swap(window);
			}
			else if(r_present != VK_SUCCESS){
				std::cerr << "vkQueuePresentKHR failed : " << r_present << std::endl;
				return false;
			}

			current_frame = (current_frame + 1) % max_frames_in_flight;

			return true;
		}

		void cleanup_swap(){
			for(auto framebuffer : swapchainFramebuffers){
				if(framebuffer != VK_NULL_HANDLE){
					vkDestroyFramebuffer(device, framebuffer, nullptr);
				}
			}
			swapchainFramebuffers.clear();

			for(auto view : swapchainImagesView){
				if(view != VK_NULL_HANDLE){
					vkDestroyImageView(device, view, nullptr);
				}
			}
			swapchainImagesView.clear();

			if(!commandBuffers.empty()){
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
				commandBuffers.clear();
			}
			
			swapchainImages.clear();		
		}

		bool re_swap(SDL_Window* window){
			int w = 0, h = 0;

			SDL_Vulkan_GetDrawableSize(window, &w, &h);
			while(w == 0 || h == 0){
				SDL_WaitEvent(nullptr);
				SDL_Vulkan_GetDrawableSize(window, &w, &h);
			}

			vkDeviceWaitIdle(device);

			if(!sur_cap_KHR()) return false;
			if(!des_formats_chk()) return false;
			if(!modes_chk()) return false;

			cleanup_swap();

			if(!re_swap_init(window)) return false;
			if(!swap_get_images()) return false;

			imagesInFlight.clear();
			imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

			if(!create_image_views()) return false;			
			if(!create_framebuffers()) return false;
			if(!create_command_buffers()) return false;
			if(!record_command_buff()) return false;

			return true;
		}

		~Vulkan_init(){
			if(device != VK_NULL_HANDLE){
				vkDeviceWaitIdle(device);
			}

			for(auto fence : inFlightFence){
				if(fence != VK_NULL_HANDLE){
					vkDestroyFence(device, fence, nullptr);
				}
			}
			inFlightFence.clear();

			for(auto sem : renderFinishedSemaphore){
				if(sem != VK_NULL_HANDLE){
					vkDestroySemaphore(device, sem, nullptr);
				}
			}
			renderFinishedSemaphore.clear();

			for(auto sem : imageAvailableSemaphore){
				if(sem != VK_NULL_HANDLE){
					vkDestroySemaphore(device, sem, nullptr);
				}
			}
			imageAvailableSemaphore.clear();

			if(commandPool != VK_NULL_HANDLE){
				vkDestroyCommandPool(device, commandPool, nullptr);
				commandPool = VK_NULL_HANDLE;
			}

			for(auto framebuffer : swapchainFramebuffers){
				if(framebuffer != VK_NULL_HANDLE){
					vkDestroyFramebuffer(device, framebuffer, nullptr);
				}
			}
			swapchainFramebuffers.clear();

			if(render_pass != VK_NULL_HANDLE){
				vkDestroyRenderPass(device, render_pass, nullptr);
				render_pass = VK_NULL_HANDLE;
			}

			for(auto view : swapchainImagesView){
				if(view != VK_NULL_HANDLE){
					vkDestroyImageView(device, view, nullptr);
				}
			}
			swapchainImagesView.clear();

			if(swapchain != VK_NULL_HANDLE){
				vkDestroySwapchainKHR(device, swapchain, nullptr);
				swapchain = VK_NULL_HANDLE;
				std::cerr << "vkDestroySwapchainKHR \n";
			}

			if(device != VK_NULL_HANDLE){
				vkDestroyDevice(device, nullptr);
				device = VK_NULL_HANDLE;
				std::cerr << "Vk device destroyed \n";	
			}
			if(surface != VK_NULL_HANDLE){
				vkDestroySurfaceKHR(instance, surface, nullptr);
				surface = VK_NULL_HANDLE;
				std::cerr << "Vk surface destroyed \n";
			}

			if(instance != VK_NULL_HANDLE){
				vkDestroyInstance(instance, nullptr);
				std::cerr << "Vk instance destroyed \n";
				instance = VK_NULL_HANDLE;
			}
		}


	private:
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice phys_dev = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue gpu_queue = VK_NULL_HANDLE;
		VkQueue present_queue = VK_NULL_HANDLE;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkRenderPass render_pass = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkSemaphore> imageAvailableSemaphore;
		std::vector<VkSemaphore> renderFinishedSemaphore;
		std::vector<VkFence> inFlightFence;
		std::vector<VkFence> imagesInFlight;
		std::vector<VkSurfaceFormatKHR> pSurfaceFormats;
		std::vector<VkPresentModeKHR> pPresentModes;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImagesView;
		std::vector<VkFramebuffer> swapchainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;
		VkSurfaceFormatKHR chosenFormat = {};
		VkPresentModeKHR chosenPresentMode = {};
		VkSurfaceCapabilitiesKHR pSurfaceCapabilities = {};
		VkPhysicalDeviceFeatures device_features{};
		VkExtent2D swapchainExtent{};
		uint32_t export_index_gpu_graphic_q = 0; // very important for later.
		uint32_t export_index_gpu_present_q = 0; // very important for later.
		uint32_t pSurfaceFormatCount = 0;
		uint32_t pPresentModeCount = 0;
		uint32_t pSwapchainImageCount = 0;
		size_t current_frame = 0;
		static constexpr size_t max_frames_in_flight = 2;
};


int main(int argc, char** argv){
	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cout << "Error : SDL_CreateWindow" << std::endl;
		return false; 	
	}


	{
		Display_window display_window;
		auto extensions = display_window.get_vk_ext();
		if(extensions.empty()) return false;

		QueueFamInd	indices;

		Vulkan_init vulkan_init;
		
		if(!vulkan_init.init(extensions)) return false;
		if(!vulkan_init.surface_vk(display_window.get_window())) return	false;
		if(!vulkan_init.select_device()) return	false;
		if(!vulkan_init.create_device()) return false;
		if(!vulkan_init.queues_vk()) return false;
		if(!vulkan_init.sur_cap_KHR()) return false;
		if(!vulkan_init.des_formats_chk()) return false;
		if(!vulkan_init.modes_chk()) return false;
		if(!vulkan_init.swap_init(display_window.get_window())) return false;
		if(!vulkan_init.swap_get_images()) return false;
		if(!vulkan_init.create_image_views()) return false;
		if(!vulkan_init.create_render_pass()) return false;
		//if(!vulkan_init.re_swap_init()) return false;
		if(!vulkan_init.create_framebuffers()) return false;
		if(!vulkan_init.create_command_pool()) return false;
		if(!vulkan_init.create_command_buffers()) return false;
		if(!vulkan_init.record_command_buff()) return false;
		if(!vulkan_init.create_sync_objects()) return false;
		

		while(display_window.run_window){
			display_window.input_from_usr();
			Frame_stats	stats = display_window.delta_time_fps();
			
			if(display_window.window_resized){
				if(!vulkan_init.re_swap(display_window.get_window())){
					std::cerr << "draw_frame failed" << std::endl;
					break;	
				}
				display_window.window_resized = false;
			}

			if(!vulkan_init.draw_frame(display_window.get_window(), display_window.window_resized)){
				std::cerr << "draw_frame failed" << std::endl;
				break;
			}
			
		}
		//std::cerr << "loop ended \n"; 
	}
		
	SDL_Quit();

    return EXIT_SUCCESS;
}