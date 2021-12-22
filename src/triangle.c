#include "triangle.h"
#include "display.h"
#include "swap.h"

/**
* Return the barycentric weights alpha, beta, and gamma for point p
**/
///////////////////////////////////////////////////////////////////////////////
//
//          A
//         /|\
//        / | \
//       /  |  \
//      /  (p)  \
//     /  /   \  \
//    / /       \ \
//   B-------------C
//
///////////////////////////////////////////////////////////////////////////////
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    // Find the vectors between the vertices ABC and point p
    vec2_t ab = vec2_sub(b, a);
    vec2_t bc = vec2_sub(c, b);
    vec2_t ac = vec2_sub(c, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t bp = vec2_sub(p, b);

    // Calcualte the area of the full triangle ABC using cross product (area of parallelogram)
    float area_triangle_abc = (ab.x * ac.y - ab.y * ac.x);

    // Weight alpha is the area of subtriangle BCP divided by the area of the full triangle ABC
    float alpha = (bc.x * bp.y - bp.x * bc.y) / area_triangle_abc;

    // Weight beta is the area of subtriangle ACP divided by the area of the full triangle ABC
    float beta = (ap.x * ac.y - ac.x * ap.y) / area_triangle_abc;

    // Weight gamma is easily found since barycentric cooordinates always add up to 1
    float gamma = 1 - alpha - beta;

    vec3_t weights = { alpha, beta, gamma };
    return weights;
}

///////////////////////////////////////////////////////////////////////////////
// Draw a triangle using three raw line calls
///////////////////////////////////////////////////////////////////////////////
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

///////////////////////////////////////////////////////////////////////////////
// Function to draw a solid pixel at position (x,y) using depth interpolation
///////////////////////////////////////////////////////////////////////////////
void draw_triangle_pixel(
    int x, int y, uint32_t color,
    vec4_t point_a, vec4_t point_b, vec4_t point_c
) {
    // Create three vec2 to find the interpolation
    vec2_t p = { x, y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    // Calculate the barycentric coordinates of our point 'p' inside the triangle
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Interpolate the value of 1/w for the current pixel
    float interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    // Adjust 1/w so the pixels that are closer to the camera have smaller values
    interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

    // Only draw the pixel if the depth value is less than the one previously stored in the z-buffer
    if (interpolated_reciprocal_w < z_buffer[(window_width * y) + x]) {
        // Draw a pixel at position (x,y) with a solid color
        draw_pixel(x, y, color);

        // Update the z-buffer value with the 1/w of this current pixel
        z_buffer[(window_width * y) + x] = interpolated_reciprocal_w;
    }
}

void draw_filled_triangle(
    int x0, int y0, float z0, float w0,
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2,
    uint32_t color
) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }

    // Create three vector points after we sort the vertices
    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, z1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with a solid color
                draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with a solid color
                draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
            }
        }
    }
}




/**
* Draw the textured pixel at position x and y using interpolation
**/
void draw_texel(
    int x, int y, uint32_t* texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
    vec2_t p = { x, y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);

    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Variables to store the interpolated values of U, V, and also 1/w for the current pixel
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // Perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
    interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
    interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

    // Also interpolate the value of 1/w for the current pixel
    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    // Now we can divide back both interpolated values by 1/w
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // Map the UV coordinate to the full texture width and height
    // Truncating within the allocated dimensions at the end of these lines is a messy hack
    // to make sure we are not trying to write to a value outside of allocated memory
    // GPU's take care of this using Fill Convention. We are doing it the old fashioned way
    // Note that this may result in some 'tears' between faces
    int tex_x = abs((int)(interpolated_u * texture_width)) % texture_width;
    int tex_y = abs((int)(interpolated_v * texture_height)) % texture_height;

    // invert 1/w so pixels that are closer to cam have smaller values
    interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

    // As long as the current pixel is in front of what is there currently
    // (i.e., depth value of this pixel is LESS than the one previously stored in z-buffer)...
    if (interpolated_reciprocal_w < z_buffer[(window_width * y) + x]) {
        // ...draw the pixel
        draw_pixel(x, y, texture[(texture_width * tex_y) + tex_x]);
        // ... and update the z-buffer value with the 1/w (1 / old z in camera space) of this current pixel
        z_buffer[(window_width * y) + x] = interpolated_reciprocal_w;
    }
}

//AFFINE MAPPING (draw texel()):
/*
void draw_texel(
    int x, int y, uint32_t* texture,
    vec2_t point_a, vec2_t point_b, vec2_t point_c,
    float u0, float v0, float u1, float v1, float u2, float v2
) {
   vec2_t point_p = { x, y };
   vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p);

   float alpha = weights.x;
   float beta = weights.y;
   float gamma = weights.z;

   // Calculate interpolation of all U and V values using barycentric weights
   float interpolated_u = (u0 * alpha) + (u1 * beta) + (u2 * gamma);
   float interpolated_v = (v0 * alpha) + (v1 * beta) + (v2 * gamma);

   // Map the UV coordinate to the full texture width and height
   int texture_x = abs((int)(interpolated_u * texture_width));
   int texture_y = abs((int)(interpolated_v * texture_height));

   // Draw the actual pixel, passing for color value the width of the texture buffer * y (to get y-coord of
   // the texture buffer) offset by x (to get x coord). This is the same little formula we use for the color buffer
   // to fetch a 2D coordinate from a 1d array.
   int texIndex = ((texture_width * texture_y) + texture_x) % (texture_width * texture_height);
   draw_pixel(x, y, texture[texIndex]);

}
*/

void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    uint32_t* texture
) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }


     // Flip the V component to account for inverted UV-coordinates (V grows downwards)
    v0 = 1.0 - v0;
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;

    // Create vector points and texture coords after we sort the vertices
    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, z1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };
    tex2_t a_uv = { u0, v0 };
    tex2_t b_uv = { u1, v1 };
    tex2_t c_uv = { u2, v2 };

    ///////////////////////////////////////////////////////
    // Render the upper part of the triangle (flat-bottom)
    ///////////////////////////////////////////////////////
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with the color that comes from the texture
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // Render the bottom part of the triangle (flat-top)
    ///////////////////////////////////////////////////////
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw our pixel with the color that comes from the texture
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }
}

/**
* Draw a textured triangle using the flat-top/flat-bottom method
**/
/*
void draw_textured_triangle(
        int x0, int y0, float z0, float w0, float u0, float v0, // including z and w because every pixel needs to carry depth
        int x1, int y1, float z1, float w1, float u1, float v1, // information to prevent affine texture mapping disortion
        int x2, int y2, float z2, float w2, float u2, float v2,
        uint32_t* texture
) {
// loop all pixels of triangle to render them based on color fetched from corresponding uv coordinate in texture array
    // sort the vertices by ascending y-coordinates
    // z and w are included for the affine mapping distortion fix
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Create vector points after sorting vertices
    vec4_t point_a = { x0, y0, z0, y0 };
    vec4_t point_b = { x1, y1, z1, y1 };
    vec4_t point_c = { x2, y2, z2, y2 };
    tex2_t a_uv = { u0, v0 };
    tex2_t b_uv = { u1, v1 };
    tex2_t c_uv = { u2, v2 };

    /////////////////////////////////////////////////////
    //// Render upper-half of triangle (flat-bottom) ////
    /////////////////////////////////////////////////////

    // Using inverse of slope because y is our dependent variable
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    // Get inverse slope of left leg (check to prevent division by 0)
    if((y1 - y0) != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    // Get inverse slpe of right leg (check to prevent division by 0)
    if((y2 - y0) != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    // Only render if delta y != 0 (prevents bug where odd edges are rendered)
    if (y1 - y0 != 0) {
        // From height (y) of triangle tip to height (y) of middle (flat-bottom) ...
        for (int y = y0; y <= y1; y++) {
            // Calculate the beginning (left-boundary) of this scanline
            int x_start = x1 + (y - y1) * inv_slope_1;
            // Calculate the end (right-boundary) of this scanline
            int x_end = x0 + (y - y0) * inv_slope_2;

            // Swap start and end if end is before start
            if(x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw pixel with color fetched from texture
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }

        }
    }

    /////////////////////////////////////////////////////
    ////   Render lower-half of triangle (flat-top)  ////
    /////////////////////////////////////////////////////

    // Using inverse of slope because y is our dependent variable
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    // Get inverse slope of left leg (check to prevent division by 0)
    if((y2 - y1) != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    // Get inverse slpe of right leg (check to prevent division by 0)
    if((y2 - y0) != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    // Only render if delta y != 0 (prevents bug where odd edges are rendered)
    if (y2 - y1 != 0) {
        // From height (y) of middle (where we just left off) to height (y) of bottom tip of triangle...
        for (int y = y1; y <= y2; y++) {
            // Calculate the beginning (left-boundary) of this scanline
            int x_start = x1 + (y - y1) * inv_slope_1;
            // Calculate the end (right-boundary) of this scanline
            int x_end = x0 + (y - y0) * inv_slope_2;

            // Swap start and end if end is before start
            if(x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw pixel with color fetched from texture
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }

        }
    }
}
*/

// AFFINE MAPPING (draw_textured_triangle()):
/*
void draw_textured_triangle(
        int x0, int y0, float u0, float v0,
        int x1, int y1, float u1, float v1,
        int x2, int y2, float u2, float v2,
        uint32_t* texture
) {
// loop all pixels of triangle to render them based on color fetched from corresponding uv coordinate in texture array
    // sort the vertices by ascending y-coordinates
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Create vector points after sorting vertices
    vec2_t point_a = { x0, y0 };
    vec2_t point_b = { x1, y1 };
    vec2_t point_c = { x2, y2 };

    /////////////////////////////////////////////////////
    //// Render upper-half of triangle (flat-bottom) ////
    /////////////////////////////////////////////////////

    // Using inverse of slope because y is our dependent variable
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    // Get inverse slope of left leg (check to prevent division by 0)
    if((y1 - y0) != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    // Get inverse slpe of right leg (check to prevent division by 0)
    if((y2 - y0) != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    // Only render if delta y != 0 (prevents bug where odd edges are rendered)
    if (y1 - y0 != 0) {
        // From height (y) of triangle tip to height (y) of middle (flat-bottom) ...
        for (int y = y0; y <= y1; y++) {
            // Calculate the beginning (left-boundary) of this scanline
            int x_start = x1 + (y - y1) * inv_slope_1;
            // Calculate the end (right-boundary) of this scanline
            int x_end = x0 + (y - y0) * inv_slope_2;

            // Swap start and end if end is before start
            if(x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw pixel with color fetched from texture
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
            }

        }
    }

    /////////////////////////////////////////////////////
    ////   Render lower-half of triangle (flat-top)  ////
    /////////////////////////////////////////////////////

    // Using inverse of slope because y is our dependent variable
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    // Get inverse slope of left leg (check to prevent division by 0)
    if((y2 - y1) != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    // Get inverse slpe of right leg (check to prevent division by 0)
    if((y2 - y0) != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    // Only render if delta y != 0 (prevents bug where odd edges are rendered)
    if (y2 - y1 != 0) {
        // From height (y) of middle (where we just left off) to height (y) of bottom tip of triangle...
        for (int y = y1; y <= y2; y++) {
            // Calculate the beginning (left-boundary) of this scanline
            int x_start = x1 + (y - y1) * inv_slope_1;
            // Calculate the end (right-boundary) of this scanline
            int x_end = x0 + (y - y0) * inv_slope_2;

            // Swap start and end if end is before start
            if(x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++) {
                // Draw pixel with color fetched from texture
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
            }

        }
    }
}
*/

