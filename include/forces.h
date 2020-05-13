#ifndef __FORCES_H__
#define __FORCES_H__

#include "scene.h"
#include "collision.h"

// Structs to hold helper data for gravity, spring, and drag creators
typedef struct gravity_params GravityParams;

typedef struct spring_params SpringParams;

typedef struct drag_params DragParams;

typedef struct coll_params CollParams;

typedef struct phys_coll_params PhysCollParams;

typedef struct gen_coll_params GenCollParams;


/**
 * Adds a Newtonian gravitational force between two bodies in a scene.
 * See https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation#Vector_form.
 * The force should not be applied when the bodies are very close,
 * because its magnitude blows up as the distance between the bodies goes to 0.
 *
 * @param scene the scene containing the bodies
 * @param G the gravitational proportionality constant
 * @param body1 the first body
 * @param body2 the second body
 */
void create_newtonian_gravity(Scene *scene, double G, Body *body1, Body *body2);

void GravityForceCreator(void *aux);

/**
 * Adds a Hooke's-Law spring force between two bodies in a scene.
 * See https://en.wikipedia.org/wiki/Hooke%27s_law.
 *
 * @param scene the scene containing the bodies
 * @param k the Hooke's constant for the spring
 * @param body1 the first body
 * @param body2 the second body
 */
void create_spring(Scene *scene, double k, Body *body1, Body *body2);

void SpringForceCreator(void *aux);

/**
 * Adds a drag force on a body proportional to its velocity.
 * The force points opposite the body's velocity.
 *
 * @param scene the scene containing the bodies
 * @param gamma the proportionality constant between force and velocity
 *   (higher gamma means more drag)
 * @param body the body to slow down
 */
void create_drag(Scene *scene, double gamma, Body *body);

void DragForceCreator(void *aux);

/**
 * Adds a ForceCreator to a scene that calls a given CollisionHandler
 * each time two bodies collide.
 * This generalizes create_destructive_collision() from last week,
 * allowing different things to happen when bodies collide.
 * The handler is passed the bodies, the collision axis, and an auxiliary value.
 * It should only be called once while the bodies are still colliding.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 * @param handler a function to call whenever the bodies collide
 * @param aux an auxiliary value to pass to the handler
 * @param freer if non-NULL, a function to call in order to free aux
 */
void create_collision(
    Scene *scene,
    Body *body1,
    Body *body2,
    CollisionHandler handler,
    void *aux,
    FreeFunc freer
);

void CollisionCreator(void* aux);

/**
 * Adds a ForceCreator to a scene that destroys two bodies when they collide.
 * The bodies should be destroyed by calling body_remove().
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 */
void create_destructive_collision(Scene *scene, Body *body1, Body *body2);

void DestructiveCollisionCreator(void *aux);

/**
 * Adds a ForceCreator to a scene that applies impulses
 * to resolve collisions between two bodies in the scene.
 * This should be represented as an on-collision callback
 * registered with create_collision().
 *
 * You may remember from project01 that you should avoid applying impulses
 * multiple times while the bodies are still colliding.
 * You should also have a special case that allows either body1 or body2
 * to have mass INFINITY, as this is useful for simulating walls.
 *
 * @param scene the scene containing the bodies
 * @param elasticity the "coefficient of restitution" of the collision;
 * 0 is a perfectly inelastic collision and 1 is a perfectly elastic collision
 * @param body1 the first body
 * @param body2 the second body
 */
void create_physics_collision(
    Scene *scene, double elasticity, Body *body1, Body *body2
);

void PhysicsCollisionCreator(void *aux);

Vector unit_vector(Vector v);

double distance(Vector loc1, Vector loc2);

#endif // #ifndef __FORCES_H__
