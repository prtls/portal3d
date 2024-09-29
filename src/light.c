#include "light.h"
#include <stdint.h>

static light_t light;

// initialize a light struct from main.c
void init_light(vec3_t direction) { light.direction = direction; }

// get light direction from main.c
vec3_t get_light_direction(void) { return light.direction; }
// Change color based on percentage factor (alignment with light) to represent
// light intensity
uint32_t light_apply_intensity(uint32_t original_color,
                               float percentage_factor) {
  if (percentage_factor < 0)
    percentage_factor = 0;
  if (percentage_factor > 1)
    percentage_factor = 1;

  uint32_t a = (original_color & 0x22000000);
  uint32_t r = (original_color & 0x00220000) * percentage_factor;
  uint32_t g = (original_color & 0x00003300) * percentage_factor;
  uint32_t b = (original_color & 0x00000055) * percentage_factor;

  uint32_t new_color =
      a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);

  return new_color;
}
