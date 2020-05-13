#include "body.h"
#include "polygon.h"
#include <stdlib.h>
#include <math.h>

struct body {
  List *shape;
  void *info;
  FreeFunc info_freer;
  double mass;
  RGBColor color;
  Vector centroid;
  Vector velocity;
  Vector force;
  Vector impulse;
  bool to_remove;
};

Body *body_init(List *shape, double mass, RGBColor color) {
  Body* body = malloc(sizeof(Body));
  body->shape = shape;
  body->mass = mass;
  body->color = color;
  if (list_size(shape) > 0) {
    body->centroid = polygon_centroid(shape);
  }
  else {
    body->centroid = (Vector){.x = 0, .y = 0};
  }
  body->velocity = VEC_ZERO;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->to_remove = false;
  body->info = (void*)list_init(0, free);
  body->info_freer = (FreeFunc)list_free;
  return body;
}

Body *body_init_with_info(List *shape, double mass, RGBColor color, void *info, FreeFunc info_freer) {
  Body *body = body_init(shape, mass, color);
  body->info = info;
  body->info_freer = info_freer;
  return body;
}

void body_free(Body *body) {
  list_free(body->shape);
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  free(body);
}

List *body_get_shape(Body *body) {
  List *shape_copy = list_init(list_size(body->shape), free);
  for (int i = 0; i < list_size(body->shape); i++) {
    Vector to_add = *(Vector*)list_get(body->shape, i);
    Vector *to_add_pointer = malloc(sizeof(Vector));
    to_add_pointer->x = to_add.x;
    to_add_pointer->y = to_add.y;
    list_add(shape_copy, (void*)to_add_pointer);
  }
  return shape_copy;
}

Vector body_get_centroid(Body *body) {
  return body->centroid;
}

Vector body_get_velocity(Body *body) {
  return body->velocity;
}

RGBColor body_get_color(Body *body) {
  return body->color;
}

void *body_get_info(Body *body) {
  return body->info;
}

double body_get_mass(Body *body) {
  return body->mass;
}

void body_set_centroid(Body *body, Vector x) {
  Vector old_centroid = body->centroid;
  body->centroid = x;
  polygon_translate(body->shape, vec_subtract(x, old_centroid));
}

void body_set_velocity(Body *body, Vector v) {
  body->velocity = v;
}

void body_set_rotation(Body *body, double angle) {
  polygon_rotate(body->shape, angle, body->centroid);
}

void body_add_force(Body *body, Vector force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(Body *body, Vector impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_tick(Body *body, double dt) {
  Vector init_velocity = body->velocity;

  // p = mv
  if (body->mass != INFINITY) {
    Vector momentum = vec_multiply(body->mass, body->velocity);
    momentum = vec_add(momentum, body->impulse);
    body->velocity = vec_multiply(1 / body->mass, momentum);
  }

  // F = ma
  Vector acceleration = vec_multiply(1 / body->mass, body->force);
  body->velocity = vec_add(body->velocity, vec_multiply(dt, acceleration));

  body->impulse = VEC_ZERO;
  body->force = VEC_ZERO;

  Vector average_velocity = vec_multiply(.5, vec_add(init_velocity, body->velocity));

  Vector translation = vec_multiply(dt, average_velocity);
  body_set_centroid(body, vec_add(body->centroid, translation));
}

void body_remove(Body *body) {
  body->to_remove = true;
}

bool body_is_removed(Body *body) {
  return body->to_remove;
}
