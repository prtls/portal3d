#include "display.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static uint32_t* color_buffer = NULL;
static float* z_buffer = NULL;

static SDL_Texture* color_buffer_texture = NULL;
static int window_width = 800;
static int window_height = 600;

static int render_method;
static int cull_method;

int get_window_width(void) {
    return window_width;
}

int get_window_height(void) {
    return window_height;
}
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

    //allocate the required memory for the color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    //allocate the required memory for the depth buffer
    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	// Create SDL texture that is used to display the color buffer
	// Remember, the color buffer is just a data structure that holds the pixel values,
	// while the texture is the actual thing that will be displayed, so we need to
	// copy our color buffer into it
	color_buffer_texture = SDL_CreateTexture(
		renderer, //renderer that will be responsible for displaying this texture
		SDL_PIXELFORMAT_RGBA32, //choose an appropriate pixel format
		SDL_TEXTUREACCESS_STREAMING, //pass this when we're going to continuously stream this texture
		window_width, //width of the actual texture (not always window width)
		window_height //height of actual texture (not always window height)
	);

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
    for (int i = 0; i < window_width * window_height; i++) {
        color_buffer[i] = color;
    }
}


/**
 * Clear the depth buffer (to be called before displaying a new frame)
 */
void clear_z_buffer(void) {
    for (int i = 0; i < window_width * window_height; i++) {
            z_buffer[i] = 1.0;
	}
}

/**
 *
*/
void draw_pixel(int x, int y, uint32_t color) {
	if(x < 0 || x >= window_width || y < 0 || y >=  window_height) {
        return;
    }
    color_buffer[(window_width * y) + x] = color;
}

/**
 *
*/
void draw_thick_pixel(int x, int y, uint32_t color) {
	if(x >= 0 && x < window_width && y >= 0 && y < window_height) {
		color_buffer[(window_width * y) + x] = color;
		color_buffer[(window_width * (y+1)) + x] = color;
		color_buffer[(window_width * y) + (x+1)] = color;
		color_buffer[(window_width * (y-1)) + x] = color;
		color_buffer[(window_width * y) + (x-1)] = color;
		color_buffer[(window_width * (y+1)) + (x+1)] = color;
		color_buffer[(window_width * (y-1)) + (x+1)] = color;
		color_buffer[(window_width * (y-1)) + (x-1)] = color;
		color_buffer[(window_width * (y+1)) + (x-1)] = color;
	}
}

void draw_horizon() {
//TODO: Make this not horrifically retarded (use switch(?), simplify checked values and make higher resolution color steps)
	uint32_t color = 0xFF000000;
	for (int y = 0; y < window_height; y++) {
        switch(y%5) {
            case 0: (color = 0xFF330000);
                    break;
        }

		for (int x = 0; x < window_width; x++) {
			color_buffer[(window_width * y) + x] = color;
		}
	}
    // ORIGINAL 'LIGHT' IMPLEMENTATION
    /*for (int y = 0; y < window_height; y++) {
		if (y >= window_height/10 && y <= 2*(window_height/10)) {
			color = 0xFFAA9933;
		}

		else if (y >= 2*(window_height/10) && y <= 3*(window_height/10)) {
			color = 0xFFAA9955;
		}

		else if (y >= 3*(window_height/10) && y <= 4*(window_height/10)) {
			color = 0xFFAA9977;
		}

		else if (y >= 4*(window_height/10) && y <= 5*(window_height/10)) {
			color = 0xFFAA9999;
		}

		else if (y >= 5*(window_height/10) && y <= 6*(window_height/10)) {
			color = 0xFFAA99BB;
		}

		else if (y >= 6*(window_height/10) && y <= 7*(window_height/10)) {
			color = 0xFF0099FF;
		}

		else if (y >= 7*(window_height/10) && y <= 8*(window_height/10)) {
			color = 0xFF0077EE;
		}

		else if (y >= 8*(window_height/10) && y <= 9*(window_height/10)) {
			color = 0xFF0055DD;
		}

		else if (y >= 9*(window_height/10) && y <= (window_height/10)) {
			color = 0xFF0033CC;
		}

		for (int x = 0; x < window_width; x++) {
			color_buffer[(window_width * y) + x] = color;
		}
	}
*/


}

/**
 * set render method (textured, wireframe, solid)
 */
void set_render_method(int method) {
    render_method = method;
}

/**
 * enable or disable backface culling
 */
void set_cull_method(int method) {
    cull_method = method;
}

/**
 * check if backface culling is enabled
 */
bool is_cull_backface(void) {
    return cull_method == CULL_BACKFACE;
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

/**
 * Draw a line to the color buffer using a DDA (digital differential analyzer) algorithm
 *
 * @param: x0 : starting point x value
 * @param: y0 : starting point y value
 * @param: x1 : ending point x value
 * @param: x2 : ending point y value
 * @param: color : color to draw line in
 */
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
	int delta_x = (x1 - x0);
	int delta_y = (y1 - y0);

	int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

	// Find how much we should increment in both x and y each step
	float x_inc = delta_x / (float) side_length;
	float y_inc = delta_y / (float) side_length;

	float current_x = x0;
	float current_y = y0;

	for (int i = 0; i <= side_length; i++) {
		//draw_pixel(round(current_x), round(current_y), color);
		draw_pixel(round(current_x), round(current_y), color); //experimenting. delete when continuing course
		current_x += x_inc;
		current_y += y_inc;
	}
}

void destroy_window(void) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

