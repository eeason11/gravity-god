#ifndef _POLYGON_HELPER_H_
#define _POLYGON_HELPER_H_

#include "list.h"
#include "vector.h"
#include "polygon.h"

/**
 * Computes the points of a star based on its location, number of vertices, and
 * radius
 */
List *polygon_points(Vector center, int vertices, int radius, double star_factor);

/**
 * Computes the points of an ellipse based on its location, number of vertices,
 * x radius, and y radius
 */
List *ellipse_points(Vector center, int vertices, int x_radius, int y_radius);

/**
 * Computes the points of a incomplete circle based on number of vertices,
 * radius, center, and start and end angles of the missing slice
 */
List *pie(Vector center, int vertices, int radius, double angle_start, double angle_end);

List *rectangle_points(Vector center, double width, double height);

#endif // #ifndef _POLYGON_HELPER_H_
