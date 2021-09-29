#include "triangle.h"
#include "display.h"
#include "swap.h"


void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {

    // find the 2 slopes (from the two triangle legs)
    // note that since x is the dependent variable, we want delta_x/delta_y (inverse slope)
    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // initialize x_start and x_end at top vertex (x0, y0)
    float x_start = x0;
    float x_end = x0;

    // loop all scanlines from top to bottom
    for (int y = y0; y <= y2; y++) {
        draw_line(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }

}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {

    // find the 2 slopes (from the two triangle legs)
    // note that since x is the dependent variable, we want delta_x/delta_y (inverse slope)
    float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

    // initialize x_start and x_end at base (bottom) vertex (x2, y2)
    float x_start = x2;
    float x_end = x2;

    // loop all scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
        draw_line(x_start, y, x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // sort verticies by ascending y-coord
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 == y2) {

        // in this case we can simply draw the flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);

    } else if (y0 == y1) {

        // in this case we can simply raw the flat-top triangle
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);

    } else {

        // calculate the new vertex (Mx,My) using triangle similarity
        int My = y1;
        int Mx = ((float)((x2-x0) * (y1-y0)) / (float)(y2-y0)) + x0;

        // draw flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

        // draw flat-top triangle
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

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


/**
* Draw the textured pixel at position x and y using interpolation
**/
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

/**
* Draw a textured triangle using the flat-top/flat-bottom method
**/
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

