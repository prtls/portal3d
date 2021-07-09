//STANDARD INCLUDES
#include <stdio.h>
#include <stdint.h> //necessary?
#include <stdbool.h>
#include <SDL2/SDL.h>

//USER-DEFINED INCLUDES
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

//MACRO DEFINITIONS

//GLOBAL VARIABLES
bool is_running = false;
int previous_frame_time = 0;
int fov_factor = 640;

//Array of triangles that should be rendered frame by frame
triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };


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

	load_cube_mesh_data();

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
	//block program until we have reached the millisecond duration we designated for 1 frame
	//in FRAME_TARGET_TIME (for 30 fps that's 33.333ms)
	//this locks our animation to our constant framerate so that fps is machine independent:
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	//only delay execution if we are running too fast:
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	//calculate how many ms have passed since last frame
	previous_frame_time = SDL_GetTicks(); //how many ms have passed since SDL_init()

	//initialize the array of triangles to render
	triangles_to_render = NULL;

	//rotate cube
	mesh.rotation.y += 0.01;
	mesh.rotation.x += 0.01;
	mesh.rotation.z += 0.01;

	//loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);
	for(int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];
		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		triangle_t projected_triangle;

		//loop all 3 vertices of this current face and apply transformations
		for(int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			//translate the vertex away from the camera (in Z)
			transformed_vertex.z -= camera_position.z;


			//project the current vertex
			vec2_t projected_point = perspectiveProject(transformed_vertex);

			//scale and translate the projected points to the middle of screen
			projected_point.x += (window_width / 2);
			projected_point.y += (window_height / 2);

			projected_triangle.points[j] = projected_point;

		}

	//save the projected triangles in the array of triangles to render
	array_push(triangles_to_render, projected_triangle);

	}

}

void render(void) {
    draw_grid(0xFF222222, 0x00000000);

    //loop all projected points and render them
    int num_triangles = array_length(triangles_to_render);
    for(int i = 0; i < num_triangles; i++) {
	    triangle_t triangle = triangles_to_render[i];
	    // draw vertex points
	    draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFF0099FF);
	    draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFF0099FF);
	    draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFF0099FF);

	    // draw unfilled triangle
	    draw_triangle(
		    triangle.points[0].x,
		    triangle.points[0].y,
		    triangle.points[1].x,
		    triangle.points[1].y,
		    triangle.points[2].x,
		    triangle.points[2].y,
		    0xFF00FF77
	    );
    }
    //clear array of triangles to render every frame loop
    array_free(triangles_to_render);


    render_color_buffer();
    clear_color_buffer(0xFF000000);

    //actually present the color buffer
    SDL_RenderPresent(renderer);
}

// free the memory that was dynamically allocated by program
void free_resources(void) {
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
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
    free_resources();

    return 0;
}

