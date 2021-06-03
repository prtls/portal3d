//STANDARD INCLUDES
#include <stdio.h>
#include <stdint.h> //necessary?
#include <stdbool.h>
#include <SDL2/SDL.h>

//USER-DEFINED INCLUDES
#include "../include/display.h"

//GLOBAL VARIABLES
bool is_running = false;


/**
 * Allocate required memory for color buffer and create the SDL texture
 * that is used to display it
 */
void setup(void) {
	//allocate the required bytes in memory for the color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

	// Create SDL texture that is used to display the color buffer
	// remember, the color buffer is just a data structure that holds the pixel values,
	// while the texture is the actual thing that will be displayed, so we need to
	// copy our color buffer into it
	color_buffer_texture = SDL_CreateTexture(
		renderer, //renderer that will be responsible for displaying this texture
		SDL_PIXELFORMAT_ARGB8888, //choose an appropriate pixel format
		SDL_TEXTUREACCESS_STREAMING, //pass this when we're going to continuously stream this texture
		window_width, //width of the actual texture (not always window width)
		window_height //height of actual texture (not always window height)
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

void render(void) {
    //set color of renderer to paint to screen
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    //clear the whole window with the renderer of that color
    SDL_RenderClear(renderer);

    //currently just playing around
    render_color_buffer();
    draw_grid(0xFF00FF00, 0x00000000);
    draw_rect(20, 20, 55, 55, 0xFF0000FF);
    draw_pixel(30, 30, 0xFFFFFF00);

    //actually present the color buffer
    SDL_RenderPresent(renderer);
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

