#pragma once

#include <SDL2/SDL.h>

struct Frame_stats{
	double fps;
	double avg_dt;
	double dt;
};


class Display_window{
	public:
		SDL_Window* get_window() const;
		
		Display_window();
		~Display_window();
	
		void input_from_usr();
		Frame_stats delta_time_fps();

		bool is_running();
		bool was_resized();
		void reset_resized_flag();

	private:
		SDL_Window* window = nullptr;
		bool run_window = true;
		bool window_resized = false;
		int window_width = 1280;
		int window_height = 960;
		
};