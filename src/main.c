//STANDARD INCLUDES
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <math.h>

//USER-DEFINED INCLUDES
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "triangle.h"

//MACRO DEFINITIONS
#define M_PI 3.14159

//GLOBAL VARIABLES
bool is_running = false;
int previous_frame_time = 0;

//TODO: make a settings.h file to store these enums

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE
} cull_method;

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
} render_method;

//Array of triangles to be rendered frame by frame
triangle_t* triangles_to_render = NULL;

//a 3d vector struct containin coordinates of the camera
vec3_t camera_position= { 0, 0, 0 };

mat4_t proj_matrix;

/**
 * Allocate required memory for color buffer and create the SDL texture
 * that is used to display it
 */
void setup(void) {
    // initialize render mode
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

	//allocate the required memory for the color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

	// Create SDL texture that is used to display the color buffer
	// Remember, the color buffer is just a data structure that holds the pixel values,
	// while the texture is the actual thing that will be displayed, so we need to
	// copy our color buffer into it
	color_buffer_texture = SDL_CreateTexture(
		renderer, //renderer that will be responsible for displaying this texture
		SDL_PIXELFORMAT_ARGB8888, //choose an appropriate pixel format
		SDL_TEXTUREACCESS_STREAMING, //pass this when we're going to continuously stream this texture
		window_width, //width of the actual texture (not always window width)
		window_height //height of actual texture (not always window height)
	);

    // initialize perspective projection matrix
    float fov = M_PI/3.0; // 60 deg in radians
    float aspect_ratio = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect_ratio, znear, zfar);

    // Manually load hardcoded texture data from static array
    mesh_texture = (uint32_t*)REDBRICK_TEXTURE;
    texture_width = 64;
    texture_height = 64;

	//load the cube values in the mesh data structure
	load_cube_mesh_data();

	//load_obj_file_data("./assets/f22.obj");
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
            /*
            // Mesh movement keys
            if (event.key.keysym.sym == SDLK_UP)
                mesh.rotation.x += 0.1;
            if (event.key.keysym.sym == SDLK_DOWN)
                mesh.rotation.x -= 0.1;
            if (event.key.keysym.sym == SDLK_RIGHT)
                mesh.rotation.y += 0.1;
            if (event.key.keysym.sym == SDLK_LEFT)
                mesh.rotation.y -= 0.1;

           // TODO: add combined rotation (if holding down vertical and horizontal keys together)
            */
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            if (event.key.keysym.sym == SDLK_1)
                render_method = RENDER_WIRE;
            if (event.key.keysym.sym == SDLK_2)
                render_method = RENDER_WIRE_VERTEX;
            if (event.key.keysym.sym == SDLK_3)
                render_method = RENDER_FILL_TRIANGLE;
            if (event.key.keysym.sym == SDLK_4)
                render_method = RENDER_FILL_TRIANGLE_WIRE;
            if (event.key.keysym.sym == SDLK_5)
                render_method = RENDER_TEXTURED;
            if (event.key.keysym.sym == SDLK_6)
                render_method = RENDER_TEXTURED_WIRE;
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            break;
    }
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

	// Change mesh scale/rotation values on every frame
	//mesh.rotation.y += 0.01;
	mesh.rotation.x += 0.07;
	mesh.rotation.y += 0.01;
	mesh.rotation.z += 0.01;
    //mesh.scale.x +=0.002;
    //mesh.scale.y +=0.002;
    //mesh.translation.x += 0.01;
    mesh.translation.z = 5.0;

    // Create scale, translation and rotation matrices that will be used to multiply the mesh vertices,
    // passing in the corresponding values (that are changing over time) in the mesh struct of the corresponding object
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	//loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);
	for(int i = 0; i < num_faces; i++) {
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		//loop all 3 vertices of this current face and apply transformations
		for(int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Create a World Matrix combining scale, rotation and translation matrices
            // Since matrix multiplication is not commutative, order matters! (scale, rotate, translate)
            mat4_t world_matrix = mat4_identity();
            // multiply w_m by scale to store scale scalars within it
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            // multiply w_m by rotation matrices to store rotation scalars within it
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            // multiply w_m by translation matrix to store translation scalars within it
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // Multiply world matrix by the original vector
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            //Save this transformed vertex (after being scaled/translated/rotated) in the array of transformed vertices
			transformed_vertices[j] = transformed_vertex;
		}

        //label each vertex of this given triangle for the sake of simplicity
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

        //culling step 1: find vectors B-A and C-A
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        //culling step 2: take their cross product and find the perpendicular normal
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        //culling step 3: find the camera ray vector by subtracting camera position from point A
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        //culling step 4: take dot product between normal and camera ray,
        //if the dot product is < 0, face is pointing away from camera, do not display the face
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // Backface culling (if enabled by user)
        if (cull_method == CULL_BACKFACE) {

            //if the face normal is pointing away from camera ray...
            if (dot_normal_camera < 0) {
                //...bypass the following section that would normally project this face
                continue;
            }
        }

        vec4_t projected_points[3];

		//loop all vertices of triangles NOT excluded by backface culling and finally project them
		for(int j = 0; j < 3; j++) {

			// project the current vertex (multiply it by the projection matrix)
			projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // scale into view using window dimensions
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height/ 2.0);

            // On-screen y coordinates are processed in the opposite direction in which they are
            // read in from .obj files, so we will invert y coordinates here
            projected_points[j].y *= -1;

			// scale and translate the projected points to the middle of screen
			projected_points[j].x += (window_width / 2.0);
			projected_points[j].y += (window_height / 2.0);

		}

        // Calculate the average depth of each face based on their respective vertices after transformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        // Calculate shade intensity baed on how aligned the face normal and light normal
        float light_intensity_factor = -vec3_dot(normal, light.direction);

        // Calculate triangle color based on light angle
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

        // Now using the data we created, we actually create the triangle to project
        triangle_t projected_triangle = {
            // assign triangle points (taken from the points we just processed (projected))
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y }
            },
            // assign triangle UV texture coordinates (taken from this object's mesh's face struct)
            .texcoords = {
                { mesh_face.a_uv.u, mesh_face.a_uv.v },
                { mesh_face.b_uv.u, mesh_face.b_uv.v },
                { mesh_face.c_uv.u, mesh_face.c_uv.v }
            },
            // assign this triangle's color
            .color = triangle_color,

            // assign average depth (for painters algo)
            .avg_depth = avg_depth
        };

	//save the projected triangles in the array of triangles to render
	array_push(triangles_to_render, projected_triangle);

	}

    // Sort the triangles to render by their avg_depth
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        for (int j = i; j < num_triangles; j++) {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
                // Swap the triangles positions in the array
                triangle_t temp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp;
            }
        }
    }

}

void render(void) {
    SDL_RenderClear(renderer);
    draw_grid(0xFF000000, 0xFF001122);
    //draw_horizon();


    //loop all projected points and render them
    int num_triangles = array_length(triangles_to_render);
    for(int i = 0; i < num_triangles; i++) {
	    triangle_t triangle = triangles_to_render[i];

        // if render mode is set to either fill or fill+wireframe...
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            // draw filled triangle
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                triangle.color
            );
        }

        // if render mode is set to either wireframe, wireframe+vertices fill+wireframe or textured+fireframe...
        if (render_method == RENDER_WIRE|| render_method == RENDER_WIRE_VERTEX|| render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE) {
            // draw unfilled triangle
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFF999999
            );
        }

        // if render mode is set to texture or texture+wireframe...
        if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE) {
            // draw textured triangle
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
                mesh_texture
            );

        }

        // if render mode is set to wireframe+vertices, render little rectangles at each vertex
        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
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

