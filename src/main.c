//STANDARD INCLUDES
#include <stdio.h>
#include <stdint.h> //necessary?
#include <stdbool.h>
#include <SDL2/SDL.h>

//USER-DEFINED INCLUDES
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

//GLOBAL VARIABLES
bool is_running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;

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
    //Create an SDL Window
    //args: window title, top left x/y pos, size, custom flags
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
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
 * (description for setup)
 */
void setup(void) {
	//allocate the required bytes in memory for the color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * WINDOW_WIDTH * WINDOW_HEIGHT);

	// Create SDL texture that is used to display the color buffer
	// remember, the color buffer is just a data structure that holds the pixel values,
	// while the texture is the actual thing that will be displayed, so we need to
	// copy our color buffer into it
	color_buffer_texture = SDL_CreateTexture(
		renderer, //renderer that will be responsible for displaying this texture
		SDL_PIXELFORMAT_ARGB8888, //choose an appropriate pixel format
		SDL_TEXTUREACCESS_STREAMING, //pass this when we're going to continuously stream this texture
		WINDOW_WIDTH, //width of the actual texture (not always window width)
		WINDOW_HEIGHT //height of actual texture (not always window height)
	);
}

/**
 * Read events from keyboard
 */
void process_input(void) {
    //initialize event and pollevent objects needed to read events
    SDL_Event event;
    SDL_PollEvent(&event);

    //poll for SDL_QUIT (x button) and escape
    //close program if received
    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            break;
    }
}

void update(void) {
    // TODO:
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
		(int)(WINDOW_WIDTH * sizeof(uint32_t)) //texture pitch (size, in bytes, of each row)
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
	for (int y = 0; y < WINDOW_HEIGHT; y++) {
		for (int x = 0; x < WINDOW_WIDTH; x++ ) {
			color_buffer[(WINDOW_WIDTH * y) + x] = color;
		}
	}
}

/**
 * Clear the color buffer (to be called before displaying a new frame)
 *
 * @param  color: color value to clear individual pixels with
 */
void draw_grid(uint32_t color1, uint32_t color2) {
	for (int y = 0; y < WINDOW_HEIGHT; y++) {
		for (int x = 0; x < WINDOW_WIDTH; x++ ) {
			if(x % 10 == 0)
				color_buffer[(WINDOW_WIDTH * y) + x] = color1;
			else
				color_buffer[(WINDOW_WIDTH * y) + x] = color2;
		}
	}
}

void render(void) {
    //set color of renderer to paint to screen
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    //clear the whole window with the renderer of that color
    SDL_RenderClear(renderer);

    render_color_buffer();
    draw_grid(0xFF00FF00, 0x00000000);

    //actually present the color buffer
    SDL_RenderPresent(renderer);
}

void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(void) {
    //use boolean flag from initialize_window() to set is_running flag
    is_running = initialize_window();

    //allocate memory for and create required structures
    setup();

    //our game loop
    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}

