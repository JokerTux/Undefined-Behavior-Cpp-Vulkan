/*
#############################################################################################################
	TODO :
		 -check if os = linux || win 
		 -create a window for each os
#############################################################################################################
*/

#include <iostream>
#include "SDL_window.h"


Display_window::Display_window(){
	window = SDL_CreateWindow("My first vulkan - mesh", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, (SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN));

	if(!window){
		std::cerr << "Error : SDL_CreateWindow :" << SDL_GetError() << std::endl;
		window = nullptr;
	}
}

void Display_window::input_from_usr(){
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

		if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
			window_resized = true;
		}
	}
}

bool Display_window::get_window_state(){
	return run_window;
}

Frame_stats Display_window::delta_time_fps(){
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

SDL_Window* Display_window::get_window() const{
	return window;
}

bool Display_window::is_running(){
	return run_window;
}

bool Display_window::was_resized(){
	return window_resized;
}

void Display_window::reset_resized_flag(){
	window_resized = false;
}

Display_window::~Display_window(){
	if(window != nullptr){
		SDL_DestroyWindow(window);
		window = nullptr;
		std::cout << "Window has been destroyed" << std::endl;
	} 
}
