//STANDARD INCLUDES
#include <stdio.h>
#include <stdint.h> //necessary?
#include <stdbool.h>
#include <SDL2/SDL.h>

//USER-DEFINED INCLUDES
#include "display.h"
#include "vector.h"

//GLOBAL VARIABLES
bool is_running = false;
int fov_factor = 204;
vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };

#define NUM_POINTS (9*9*9)
vec3_t cube_points[NUM_POINTS]; //9x9x9 cube
vec2_t projected_points[NUM_POINTS];

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

	int point_count = 0;

	// start loading array of vectors
	// from -1 to 1 (in this 9x9x9 cube)
	for(float x = -1; x <= 1; x += 0.25) {
		for (float y = -1; y<= 1; y += 0.25) {
			for (float z = -1; z <= 1; z += 0.25) {
				vec3_t new_point;
				new_point.x = x;
				new_point.y = y;
				new_point.z = z;
				cube_points[point_count++] = new_point;
			}
		}
	}
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

/**
 * Applies perspective divide on x and y to achieve perspective
 * projection
 */
vec2_t perspectiveProject(vec3_t point) {
	vec2_t projected_point = {
		.x = (fov_factor * point.x) / point.z, //<-- persp divide
		.y = (fov_factor * point.y) / point.z, //<-- persp divide
		};
	return projected_point;
}

/**
 * receives 3d vector and returns projected 2d point
 * (effectively just eliminates z. no perspective)
 */
vec2_t orthoProject(vec3_t point) {
	vec2_t projected_point = {
		.x = (fov_factor * point.x),
		.y = (fov_factor * point.y),
		};
	return projected_point;
}


void update(void) {
	cube_rotation.y += 0.1;
	cube_rotation.x += 0.1;
	cube_rotation.z += 0.1;

   for( int i = 0; i < NUM_POINTS; i++) {
	   vec3_t point = cube_points[i];
	   vec3_t transformed_point = vec3_rotate_y(point, cube_rotation.y);
		 transformed_point = vec3_rotate_x(transformed_point, cube_rotation.x);
		 transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);
	   //translate the points away from the camera
	   transformed_point.z -= camera_position.z;
	   //project the point orthographically
	   vec2_t projected_point = perspectiveProject(transformed_point);
	   //save projected 2D vector in array of projected points
	   projected_points[i] = projected_point;
   }
}

void render(void) {
    draw_grid(0xFF222222, 0x00000000);

    //loop all projected points and render them
    for(int i = 0; i < NUM_POINTS; i++) {
	    vec2_t projected_point = projected_points[i];
	    draw_rect(
		projected_point.x + (window_width/2),
		projected_point.y + (window_height/2),
		2,
		2,
		0xFF0055FF
	    );
    }

    render_color_buffer();
    clear_color_buffer(0xFF000000);

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

