#include "forces.h"
#include "polygon.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define BOUNDING_BOX_WIDTH 150

double max(double a, double b) {
  if (a > b) {
    return a;
  }
  return b;
}

double min(double a, double b) {
  if (a < b) {
    return a;
  }
  return b;
}

void get_projection(List* points, Vector line, Vector **min, Vector **max) {
  for (size_t i = 0; i < list_size(points); i++) {
    Vector point = vec_multiply(vec_dot(*((Vector*)list_get(points, i)), line), line);
    if (point.x < (*min)->x || (point.x == (*min)->x && point.y < (*min)->y)) {
      **min = point;
    }
    if (point.x > (*max)->x || (point.x == (*max)->x && point.y > (*max)->y)) {
      **max = point;
    }
  }
}

double get_line_overlap(Vector min1, Vector min2, Vector max1, Vector max2) {
  Vector trans = vec_subtract(max2, min1);
  Vector anchor = vec_subtract(min1, trans);
  double min1_1d = distance(anchor, min1);
  double min2_1d = distance(anchor, min2);
  double max1_1d = distance(anchor, max1);
  double max2_1d = distance(anchor, max2);

  return max(0, min(max1_1d, max2_1d) - max(min1_1d, min2_1d));
}

double get_projection_overlap(Vector axis, List *shape1, List *shape2) {
  Vector *min1 = malloc(sizeof(Vector));
  min1->x = INFINITY;
  min1->y = INFINITY;
  Vector *max1 = malloc(sizeof(Vector));
  max1->x = -INFINITY;
  max1->y = -INFINITY;
  Vector *min2 = malloc(sizeof(Vector));
  min2->x = INFINITY;
  min2->y = INFINITY;
  Vector *max2 = malloc(sizeof(Vector));
  max2->x = -INFINITY;
  max2->y = -INFINITY;

  get_projection(shape1, axis, &min1, &max1);
  get_projection(shape2, axis, &min2, &max2);

  return get_line_overlap(*min1, *min2, *max1, *max2);
}

CollisionInfo find_collision(List *shape1, List *shape2) {
  if(fabs(polygon_centroid(shape1).x - polygon_centroid(shape2).x) <= BOUNDING_BOX_WIDTH) {
    double min_overlap = INFINITY;
    Vector collision_axis;

    for (size_t i = 0; i < list_size(shape1); i++) {
      Vector side;
      if (i == list_size(shape1) - 1) {
        side = vec_subtract(*((Vector*)list_get(shape1, 0)), *((Vector*)list_get(shape1, i)));
      }
      else {
        side = vec_subtract(*((Vector*)list_get(shape1, i + 1)), *((Vector*)list_get(shape1, i)));
      }

      Vector unit_parallel = unit_vector(side);
      double par_overlap = get_projection_overlap(unit_parallel, shape1, shape2);

      if (par_overlap == 0 /*|| per_overlap == 0*/) {
        return (CollisionInfo){.collided = false, .axis = VEC_ZERO};
      }
      else if (par_overlap < min_overlap) {
          min_overlap = par_overlap;
          collision_axis = unit_parallel;
      }
    }


    // Check direction of collision_axis
    Vector oneToTwo = vec_subtract(polygon_centroid(shape2), polygon_centroid(shape1));
    if ((oneToTwo.x < 0 && collision_axis.x > 0) ||
        (oneToTwo.x > 0 && collision_axis.x < 0) ||
        (oneToTwo.y < 0 && collision_axis.y > 0) ||
        (oneToTwo.y > 0 && collision_axis.y < 0)) {
          collision_axis = vec_negate(collision_axis);
        }

        return (CollisionInfo){.collided = true, .axis = collision_axis};
}
  return (CollisionInfo){.collided = false, .axis = VEC_ZERO};
}
