#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_vulkan.h>
//#define VK_USE_PLATFORM_WIN32_KHR


struct Frame_stats{
	double fps;
	double avg_dt;
	double dt;
};


struct Windows_size{
	int win_w;
	int win_h;
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
    
    Windows_size get_window_details(){
    	SDL_GetWindowSize(window, &window_width, &window_height);
    	return {window_width, window_height};
    }
	
	private:
		SDL_Window *window = nullptr; //owning.
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


		bool surface_vk(SDL_Window* window){
			if(!SDL_Vulkan_CreateSurface(window, instance,&surface)){
				std::cerr << "SDL_Vulkan_CreateSurface error : " << SDL_GetError() << '\n';
				if(surface != VK_NULL_HANDLE){
					vkDestroySurfaceKHR(instance, surface, nullptr);
					surface = VK_NULL_HANDLE;
				}

				return false;
			}
			return true;
		}

		~Vulkan_init(){
			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyInstance(instance, nullptr);
			std::cerr << "Vk instance destroyed \n";
		}


	private:
		VkInstance instance = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;	

		bool create_instance(const std::vector<const char*> &extensions){
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

};


int main(int argc, char** argv){

	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cout << "Error : SDL_CreateWindow" << std::endl;
		return 1; 	
	}


	{
		Display_window display_window;
		auto extensions = display_window.get_vk_ext();

		if(extensions.empty()){
			return 1;
		}

		Vulkan_init vulkan_init;
		vulkan_init.init(extensions);
		vulkan_init.surface_vk(display_window.get_window());

		while(display_window.run_window){
				display_window.input_from_usr();
				Frame_stats	stats = display_window.delta_time_fps();
				Windows_size w_size = display_window.get_window_details();
		}
		 
	}
		
	SDL_Quit();

    return EXIT_SUCCESS;
}
