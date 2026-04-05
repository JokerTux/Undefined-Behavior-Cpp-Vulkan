//#define SDL_MAIN_HANDLED
#include <iostream>
#include "./platform/SDL_window.h"
#include "./renderer/vk_utils.h"
#include "./renderer/instance.h"
#include "./renderer/surface.h"
#include "./renderer/device.h"
#include "./renderer/swapchain.h"
#include "./renderer/image_views.h"
#include "./renderer/render_pass.h"
#include "./renderer/framebuffers.h"
#include "./renderer/command_pool.h"
#include "./renderer/pipeline.h"
#include <vector>


int main(int argc, char** argv){
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cout << "Error : SDL_Init" << std::endl;
        return false;
    }

    {   
        Display_window display_window;
        SDL_Window* window = display_window.get_window();

        VkContext vkcontext;
        Vulkan_init vk_init;
        Init_surface init_surface;
        Select_device select_device;
        Swapchain_creation swapchain_creation;
        Image_views image_views;
        Create_render_pass create_rp;
        Create_framebuffer crt_framebuff;
        Command_pool command_pool;

        if(!window){
            SDL_Quit();
            return false;
        }
        
        if(!init_surface.get_ext(vkcontext, window)) return false;

        if(!vk_init.init(&vkcontext)){
            std::cerr << "vk_init failed" << std::endl;
            return false;
        }

        if(!init_surface.create_sur(vkcontext, window)){
            std::cerr << "create_sur failed: " << std::endl;
            return false;
        }

        if(!select_device.pick_physical_device(&vkcontext)){
            std::cerr << "pick_physical_device failed: " << std::endl;
            return false;
        }

        if(!select_device.create_logical_device(&vkcontext)){
            std::cerr << "create_logical_device failed: " << std::endl;
            return false;
        }

        if(!init_surface.surface_checks(vkcontext)){
            std::cerr << "surface_checks failed " << std::endl;
            return false;
        }
        
        if(!swapchain_creation.create_swap(&vkcontext, window)){
            std::cerr << "create_swap failed" << std::endl;
            return false;
        }

        if(!image_views.create_views(&vkcontext)){
            std::cerr << "create_views failed" << std::endl;
            return false;
        }
        
        if(!create_rp.create_ren_pass(&vkcontext)){
            std::cerr << "create_views failed" << std::endl;
            return false;
        }

        if(!crt_framebuff.create_framebuffer_info(&vkcontext, window)){
            std::cerr << "create_framebuffer_info failed" << std::endl;
        }

        if(!command_pool.create_command_pool(&vkcontext)){
            std::cerr << "create_command_pool failed" << std::endl;
        }

        if(!command_pool.create_command_buffers(&vkcontext)){
            std::cerr << "create_command_buffers failed" << std::endl;
        }

        if(!command_pool.record_command_buff(&vkcontext)){
            std::cerr << "create_command_buffers failed" << std::endl;
        }

        while(display_window.get_window_state()){
            display_window.input_from_usr();
            Frame_stats stats = display_window.delta_time_fps();
        } 
    }

    SDL_Quit();
    return 0;
}