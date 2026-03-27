//#define SDL_MAIN_HANDLED
#include <iostream>
#include "./platform/SDL_window.h"
#include "./renderer/vk_utils.h"
#include "./renderer/vk_init.h"
/*#include "./renderer/vk_render.h"
#include "./renderer/vk_utils.h"
#include "./renderer/surface.h"*/
#include <vector>

int main(int argc, char** argv){
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cout << "Error : SDL_Init" << std::endl;
        return false;
    }

    {   
        Display_window display_window;
        SDL_Window* window = display_window.get_window();
        if(!window){
            SDL_Quit();
            return false;
        }
        VkContext vkcontext;
        Vulkan_init vk_init;
        
        if(!vk_init.init(&vkcontext)) {
            std::cerr << "vk_init failed" << std::endl;
            return false;
        }

        //if(!get_ext(vkcontext, window)) return false;

        /*if(!vk_init(&vkcontext, window)) {
            std::cerr << "vk_init failed" << std::endl;
            return false;
        }

        if (!create_sur(vkcontext, window)) {
            std::cerr << "create_sur failed: " << SDL_GetError() << std::endl;
            return false;
        }

        if (!devices_search(&vkcontext)) {
            std::cerr << "devices_search failed" << std::endl;
            return false;
        }

        if (!create_swap(&vkcontext)) {
            std::cerr << "create_swap failed" << std::endl;
            return false;
        }

        while (window_export.run_window) {
            window_export.input_from_usr();
            Frame_stats stats = window_export.delta_time_fps();
        }
    }
*/  
    }
    SDL_Quit();
    return 0;
}