//USER-DEFINED INCLUDES
#include "display.h"

//GLOBAL VARIABLES
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;
int window_width = 800;
int window_height = 600;

/**
 * Initializes an SDL window and the renderer for that window
 *
 * @return  boolean to denote if window opened successfully or not
 */
bool initialize_window(void) {
    //Initialize SDL with this argument defined in SDL
    //which allows us to initialize everything we need (graphics, hardware, etc)
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

		//Use SDL to query what the fullscreen dimensions are
		SDL_DisplayMode display_mode;
		SDL_GetCurrentDisplayMode(0, &display_mode);
		window_width = display_mode.w;
		window_height = display_mode.h;

    //Create an SDL Window
    //args: window title, top left x/y pos, size, custom flags
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    // Create a SDL renderer
    //args: ptr to window it belongs to, display device (-1 = default graphics driver), custom flags
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    return true;
}


/**
 * Get the color buffer in memory and copy all of those pixel's values to
 * the texture so they can be displayed
 */
void render_color_buffer(void) {
	//copy all pixel values in color_buffer to color_buffer_texture
	SDL_UpdateTexture(
		color_buffer_texture, //the texture to be updated
		NULL, //used if we only want subsection of texture, we want the entire thing in this case
		color_buffer, //source to copy to texture
		(int)(window_width * sizeof(uint32_t)) //texture pitch (size, in bytes, of each row)
	);

	//Go and actually display these things
	//3rd and 4th args are to specify a subsection of the texture, NULL if we want entire texture
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

/**
 * Clear the color buffer (to be called before displaying a new frame)
 *
 * @param  color: color value to clear individual pixels with
 */
void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++ ) {
			color_buffer[(window_width * y) + x] = color;
		}
	}
}

/**
 *
*/
void draw_pixel(int x, int y, uint32_t color) {
	if(x >= 0 && x < window_width && y >= 0 && y < window_height)
		color_buffer[(window_width * y) + x] = color;
}

/**
 * Just a test function to draw a grid to the color buffer, will prob delete this
 *
 * @param  color1: color of grid border
 * @param  color2: color of background
 */
void draw_grid(uint32_t color1, uint32_t color2) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++ ) {
			if(x % 10 == 0 || y % 10 == 0)
				color_buffer[(window_width * y) + x] = color1;
			else
				color_buffer[(window_width * y) + x] = color2;
		}
	}
}

/**
 * Draw a rectangle to the color buffer
 *
 * @param  xPos: x-coordinate of top-left position of rectangle
 * @param  yPos: y-coordinate of top-left position of rectangle
 * @param  width: pixel width of rectangle
 * @param  height: pixel height of rectangle
 * @param  color: color of rectangle
 */
void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }
}


void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
