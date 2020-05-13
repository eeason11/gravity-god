#include "color.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

RGBColor random_color(void) {
  float red = (float)(rand() % 255) / 255.0;
  float green = (float)(rand() % 255) / 255.0;
  float blue = (float)(rand() % 255) / 255.0;

  RGBColor c = {.r = red, .g = green, .b = blue};
  return c;
}

RGBColor rainbow_color(double color_index, double max_index) {
  double freq = max_index;

  float red = (sin(freq * color_index / max_index) * 127 + 128) / 255;
  float green = (sin(freq * color_index / max_index + 2 * M_PI / 3) * 127 + 128) / 255;
  float blue = (sin(freq * color_index / max_index + 4 * M_PI / 3) * 127 + 128) / 255;

  return (RGBColor) {.r = red, .g = green, .b = blue};
}