#ifndef __COLOR_H__
#define __COLOR_H__

#define TRANSPARENT ((RGBColor) {.r = -1, .g = -1, .b = -1})

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white), unless transparent.
 */
typedef struct {
    float r;
    float g;
    float b;
} RGBColor;

/**
 * Generates a random RGBColor using the given seed for C's srand
 */
RGBColor random_color(void);

/**
 * Generates an RGBColor on some point on a spectrum of a rainbow
 * Location along the rainbow spectrum is based on the given index and maximum
 * index
 */
RGBColor rainbow_color(double color_index, double max_index);

#endif // #ifndef __COLOR_H__
