#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_rect.h>


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
			window = SDL_CreateWindow("My window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_RESIZABLE);
			if(!window){
				std::cout << "Error : SDL_CreateWindow" << std::endl; 
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

    Windows_size get_window_details(){
    	SDL_GetWindowSize(window, &window_width, &window_height);
    	return {window_width, window_height};
    }
	
	private:
		SDL_Window *window = nullptr;
		int window_width = 1280;
		int window_height = 960;
};


class Renderer{
	public:
		explicit Renderer(SDL_Window *w) : window(w){
        	create_ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    	}
   	    ~Renderer() {
    	    SDL_DestroyRenderer(create_ren);
    	    std::cout << "Renderer destroyed " << std::endl;
	    }

		SDL_Renderer *get_ren() const { return create_ren; }

	private:
		SDL_Window *window = nullptr;        // non-owning
    	SDL_Renderer *create_ren = nullptr;  // owning
};



class Display_font{
	public:
		Display_font(SDL_Renderer *r) : create_ren(r){
			font = TTF_OpenFont("./assets/fonts/Roboto.ttf",18);
			if(!font){
				std::cout << "Error : TTF_OpenFont" << std::endl;		
			}

			sur = TTF_RenderText_Solid(font, "test", White);
	    	if(sur == 0){
	    		std::cout << "Error : TTF_RenderText_Solid" << std::endl;	
	    	}

	    	texture = SDL_CreateTextureFromSurface(create_ren, sur);
			if(texture	 == 0){
	    		std::cout << "Error : TTF_RenderText_Solid" << std::endl;	
	    	}

	    }
	   
	    void display_on_screen(const Windows_size &size){
	    	if(!texture || !sur) return;
	    	message_rect.h = sur->h; 
			message_rect.w = sur->w;
	    	message_rect.x = size.win_w - message_rect.w - 10;
			message_rect.y = 0;

			SDL_RenderCopy(create_ren, texture, NULL, &message_rect);
	    }

	    void set_text(const std::string &message){
	    	if(!font) return;
    		
    		SDL_Surface *n_sur = TTF_RenderText_Solid(font, message.c_str(), White);
	    	if(!n_sur){
	    		std::cout << "Surface error : " << TTF_GetError() << std::endl;
	    		return;
	    	}

	    	SDL_Texture *n_texture = SDL_CreateTextureFromSurface(create_ren, n_sur);
	    	if(!n_texture){
	    		std::cout << "texture error : " << SDL_GetError() << std::endl;
	    		SDL_FreeSurface(n_sur);
	    		return;
	    	}

	    	if(texture) SDL_DestroyTexture(texture);
	    	if(sur) SDL_FreeSurface(sur);

	    	sur = TTF_RenderText_Solid(font, message.c_str(), White);
	 	   	texture	= SDL_CreateTextureFromSurface(create_ren, sur);

	 	   	sur = n_sur;
	 	   	texture = n_texture;
	    }

	    ~Display_font(){
	    	if(texture) SDL_DestroyTexture(texture);
	    	if(sur) SDL_FreeSurface(sur);
	    	if(font) TTF_CloseFont(font);
	    }

	private:
		TTF_Font *font = nullptr;
		SDL_Surface *sur = nullptr;
		SDL_Texture *texture = nullptr;
		SDL_Renderer *create_ren = nullptr;
		SDL_Color White = {255, 255, 255};
		SDL_Rect message_rect;
		Frame_stats	stats;
		Windows_size size;
};


int main(int argc, char** argv){
	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		std::cout << "Error : SDL_CreateWindow" << std::endl;
		return 1; 	
	}
	if(TTF_Init() != 0){
		std::cout << "Error : TTF_Init" << std::endl;
		return 1;	
	}

	{
		Display_window display_window;
		Renderer renderer(display_window.get_window());
		Display_font display_font(renderer.get_ren());
		int l_fps = -1;

		while(display_window.run_window){
			display_window.input_from_usr();
			Frame_stats	stats = display_window.delta_time_fps();

			Windows_size w_size = display_window.get_window_details();

			SDL_SetRenderDrawColor(renderer.get_ren(), 0, 0, 125, 255);
			SDL_RenderClear(renderer.get_ren());
			

			if((int)stats.fps > 0.0){
				if(stats.fps != l_fps){
					display_font.set_text("FPS : " + std::to_string((int)stats.fps));
					l_fps = stats.fps;	
				}				
				
			}	
			display_font.display_on_screen(w_size);
			SDL_RenderPresent(renderer.get_ren());
		}
	}
	TTF_Quit();
	SDL_Quit();

	return 0;
}