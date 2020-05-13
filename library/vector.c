#include "vector.h"
#include <stdlib.h>
#include <math.h>

const Vector VEC_ZERO = {
  .x = 0.0,
  .y = 0.0
};

Vector vec_add(Vector v1, Vector v2) {
  Vector sum = {
    .x = v1.x + v2.x,
    .y = v1.y + v2.y
  };

  return sum;
}

Vector vec_subtract(Vector v1, Vector v2) {
  Vector diff = {
    .x = v1.x - v2.x,
    .y = v1.y - v2.y
  };

  return diff;
}

Vector vec_negate(Vector v) {
  Vector inverse = {
    .x = -(v.x),
    .y = -(v.y)
  };

  return inverse;
}

Vector vec_multiply(double scalar, Vector v) {
  Vector product = {
    .x = scalar * v.x,
    .y = scalar * v.y
  };

  return product;
}

double vec_dot(Vector v1, Vector v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

double vec_cross(Vector v1, Vector v2) {
  return (v1.x * v2.y) - (v2.x * v1.y);
}

Vector vec_rotate(Vector v, double angle) {
  Vector rot = {
    .x = (cos(angle) * v.x) - (sin(angle) * v.y),
    .y = (sin(angle) * v.x) + (cos(angle) * v.y)
  };

  return rot;
}
