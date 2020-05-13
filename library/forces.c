#include "forces.h"
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define GRAVITY_LIMIT 1

struct gravity_params {
  double G;
  Body *body1;
  Body *body2;
};

struct spring_params {
  double k;
  Body *body1;
  Body *anchor;
};

struct drag_params {
  double gamma;
  Body *body;
};

struct coll_params {
  Body *body1;
  Body *body2;
};

struct phys_coll_params {
  double e;
  Body *body1;
  Body *body2;
  bool col_slt;
};

struct gen_coll_params {
  Body *body1;
  Body *body2;
  void *aux;
  CollisionHandler ch;
  bool col_slt;
};

double distance(Vector loc1, Vector loc2) {
  double x_dist = loc1.x - loc2.x;
  double y_dist = loc1.y - loc2.y;

  return sqrt(pow(x_dist, 2) + pow(y_dist, 2));
}

double body_distance(Body *body1, Body *body2) {
  Vector body1_loc = body_get_centroid(body1);
  Vector body2_loc = body_get_centroid(body2);
  return distance(body1_loc, body2_loc);
}

Vector unit_vector(Vector v) {
  double magnitude = sqrt(pow(v.x, 2) + pow(v.y, 2));
  return (Vector){.x = v.x / magnitude, .y = v.y / magnitude};
}

void create_newtonian_gravity(Scene *scene, double G, Body *body1, Body *body2) {
  GravityParams *aux = malloc(sizeof(GravityParams));
  aux->G = G;
  aux->body1 = body1;
  aux->body2 = body2;
  List *bodies = list_init(2, free);
  list_add(bodies, (void*)body1);
  list_add(bodies, (void*)body2);
  scene_add_bodies_force_creator(scene, (ForceCreator)GravityForceCreator, (void*)aux, bodies, (FreeFunc)free);
}

Vector get_gravity_from(Body *this, Body *other, double G) {
  double dist = body_distance(this, other);
  double force_magnitude = G * body_get_mass(this) * body_get_mass(other) / pow(dist, 2);

  Vector this_location = body_get_centroid(this);
  Vector other_location = body_get_centroid(other);
  Vector d = vec_subtract(other_location, this_location);
  double r = sqrt(pow(d.x, 2) + pow(d.y, 2));

  Vector force = vec_multiply(force_magnitude / r, d);

  if (dist < GRAVITY_LIMIT) {
    force.x = 0;
    force.y = 0;
  }

  return force;
}

void GravityForceCreator(void *aux) {
  GravityParams *g = (GravityParams*)aux;
  double G = g->G;
  Body *body1 = g->body1;
  Body *body2 = g->body2;

  Vector force_on_1 = get_gravity_from(body1, body2, G);
  body_add_force(body1, force_on_1);
  body_add_force(body2, vec_negate(force_on_1));
}

void create_spring(Scene *scene, double k, Body *body1, Body *body2) {
  SpringParams *aux = malloc(sizeof(SpringParams));
  aux->k = k;
  aux->body1 = body1;
  aux->anchor = body2;
  List *bodies = list_init(2, free);
  list_add(bodies, (void*)body1);
  list_add(bodies, (void*)body2);
  scene_add_bodies_force_creator(scene, (ForceCreator)SpringForceCreator, (void*)aux, bodies, (FreeFunc)free);
}

void SpringForceCreator(void *aux) {
  SpringParams *s = (SpringParams*)aux;
  double k = s->k;
  Body *body1 = s->body1;
  Body *anchor = s->anchor;
  double dist = body_distance(body1, anchor);
  double force_magnitude = k * dist;
  Vector anch_loc = body_get_centroid(anchor);
  Vector body1_loc = body_get_centroid(body1);
  Vector direction = unit_vector(vec_subtract(anch_loc, body1_loc));
  Vector force_on_1 = vec_multiply(force_magnitude, direction);
  Vector force_on_anch = vec_negate(force_on_1);
  body_add_force(body1, force_on_1);
  body_add_force(anchor, force_on_anch);
}

void create_drag(Scene *scene, double gamma, Body *body) {
  DragParams *aux = malloc(sizeof(DragParams));
  aux->gamma = gamma;
  aux->body = body;
  List *bodies = list_init(1, free);
  list_add(bodies, (void*)body);
  scene_add_bodies_force_creator(scene, (ForceCreator)DragForceCreator, (void*)aux, bodies, (FreeFunc)free);
}

void DragForceCreator(void *aux) {
  DragParams *d = (DragParams*)aux;
  double gamma = d->gamma;
  Body *body = d->body;
  Vector force = vec_multiply(-1 * gamma, body_get_velocity(body));
  body_add_force(body, force);
}

void create_collision(Scene *scene, Body *body1, Body *body2,
  CollisionHandler handler, void *aux, FreeFunc freer) {
    GenCollParams *auxc = malloc(sizeof(GenCollParams));
    auxc->body1 = body1;
    auxc->body2 = body2;
    auxc->aux = aux;
    auxc->ch = handler;
    List *bodies = list_init(2, free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, (ForceCreator)CollisionCreator, (void*)auxc, bodies, free);
}

void CollisionCreator(void *aux) {
  GenCollParams *ch = (GenCollParams*)aux;
  Body *body1 = ch->body1;
  Body *body2 = ch->body2;
  bool col_slt = ch->col_slt;
  CollisionHandler col_handler = ch->ch;
  CollisionInfo ci = find_collision(body_get_shape(body1), body_get_shape(body2));
  if (ci.collided && !col_slt) {
    col_handler(body1, body2, ci.axis, ch->aux);
    ch->col_slt = true;
  }
  else if (!ci.collided) {
    ch->col_slt = false;
  }
}

void create_destructive_collision(Scene *scene, Body *body1, Body *body2) {
  CollParams *aux = malloc(sizeof(CollParams));
  aux->body1 = body1;
  aux->body2 = body2;
  List *bodies = list_init(2, free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (ForceCreator)DestructiveCollisionCreator, (void*)aux, bodies, free);
}

void DestructiveCollisionCreator(void *aux) {
  CollParams *c = (CollParams*)aux;
  Body *body1 = c->body1;
  Body *body2 = c->body2;
  if(find_collision(body_get_shape(body1), body_get_shape(body2)).collided) {
    body_remove(body1);
    body_remove(body2);
  }
}

void create_physics_collision(Scene *scene, double elasticity, Body *body1, Body *body2) {
  PhysCollParams *aux = malloc(sizeof(PhysCollParams));
  aux->body1 = body1;
  aux->body2 = body2;
  aux->e = elasticity;
  aux->col_slt = false;
  List *bodies = list_init(2, free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, (ForceCreator)PhysicsCollisionCreator, (void*)aux, bodies, free);
}

void PhysicsCollisionCreator(void *aux) {
  PhysCollParams *ch = (PhysCollParams*)aux;
  Body *body1 = ch->body1;
  Body *body2 = ch->body2;
  double e = ch->e;
  bool col_slt = ch->col_slt;
  CollisionInfo ci = find_collision(body_get_shape(body1), body_get_shape(body2));
  if (ci.collided && !col_slt) {
    double mass1 = body_get_mass(body1);
    double mass2 = body_get_mass(body2);
    double speed1 = vec_dot(body_get_velocity(body1), ci.axis);
    double speed2 = vec_dot(body_get_velocity(body2), ci.axis);
    double reduced_mass = (mass1 * mass2) / (mass1 + mass2);
    double impulse_mag;
    if (mass1 == INFINITY) {
      impulse_mag = mass2 * (1 + e) * (speed2 - speed1);
    }
    else if (mass2 == INFINITY) {
      impulse_mag = mass1 * (1 + e) * (speed2 - speed1);
    }
    else {
      impulse_mag = reduced_mass * (1 + e) * (speed2 - speed1);
    }
    Vector impulse1 = vec_multiply(impulse_mag, ci.axis);
    Vector impulse2 = vec_negate(impulse1);
    body_add_impulse(body1, impulse1);
    body_add_impulse(body2, impulse2);
    ch->col_slt = true;
  }
  else if (!ci.collided) {
    ch->col_slt = false;
  }
}
