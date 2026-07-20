#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include "../renderer/vk_utils.h"


struct Frame_stats{
	double fps;
	double avg_dt;
	double dt;
};

class Display_window{
	public:
		Display_window();
		void input_from_usr();
		bool get_window_state();
		Frame_stats delta_time_fps();
		SDL_Window* get_window() const;
		bool is_running();
		bool was_resized();
		void reset_resized_flag();
		~Display_window();

	private:
		SDL_Window* window = nullptr;
		bool run_window = true;
		bool window_resized = false;
		int window_width = 2048;
		int window_height = 1024;		
};