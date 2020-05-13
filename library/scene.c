#include "scene.h"
#include "sdl_wrapper.h"
#include <stdlib.h>
#include <assert.h>

#define INITIAL_BODIES 20

struct scene {
  size_t num_bodies;
  List* bodies;
  List* forcers;
};

struct forcer {
  void* aux;
  ForceCreator forcer;
  List* bodies;
};

Scene *scene_init(void) {
  Scene* s = malloc(sizeof(Scene));
  assert(s);

  s->num_bodies = 0;
  s->bodies = list_init(INITIAL_BODIES, (void (*)(void*))body_free);
  assert(s->bodies);

  s->forcers = list_init(1, (FreeFunc)free);

  return s;
}

void scene_free(Scene *scene) {
  list_free(scene->bodies);
  for (int i = 0; i < list_size(scene->forcers); i++) {
    free(((Forcer*)list_get(scene->forcers, i))->bodies);
  }
  list_free(scene->forcers);
  free(scene);
}

size_t scene_bodies(Scene *scene) {
  return scene->num_bodies;
}

Body *scene_get_body(Scene *scene, size_t index) {
  assert(index >= 0 && index < scene->num_bodies);
  return (Body*)list_get(scene->bodies, index);
}

void scene_add_body(Scene *scene, Body *body) {
  list_add(scene->bodies, (void*)body);
  scene->num_bodies++;
}

// deprecated
void scene_remove_body(Scene *scene, size_t index) {
  assert(index >= 0 && index < scene->num_bodies);
  body_remove(scene_get_body(scene, index));
}

// deprecated
void scene_add_force_creator(Scene *scene, ForceCreator forcer, void *aux, FreeFunc freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, list_init(0, free), free);
}

void scene_add_bodies_force_creator(Scene *scene, ForceCreator forcer, void *aux, List *bodies, FreeFunc freer) {
  Forcer *new_forcer = malloc(sizeof(Forcer));
  new_forcer->aux = aux;
  new_forcer->forcer = forcer;
  new_forcer->bodies = bodies;
  list_add(scene->forcers, (void*)new_forcer);
}

void scene_draw_bodies(Scene *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    List *shape = body_get_shape(b);
    if (list_size(shape) >= 3) {
      sdl_draw_polygon(body_get_shape(b), body_get_color(b));
    }
  }
}

void scene_tick(Scene *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->forcers); i++) {
    Forcer *curr = (Forcer*)list_get(scene->forcers, i);
    curr->forcer(curr->aux);
  }

  for (size_t i = 0; i < scene->num_bodies; i++) {
    body_tick(scene_get_body(scene, i), dt);
  }

  scene_tick_delete_only(scene);

  scene_draw_bodies(scene);

}

void scene_tick_delete_only(Scene *scene) {
  for (size_t i = 0; i < list_size(scene->forcers); i++) {
    Forcer *curr = (Forcer*)list_get(scene->forcers, i);
    for (size_t j = 0; j < list_size(curr->bodies); j++) {
      if (body_is_removed(list_get(curr->bodies, j))) {
        Forcer *rem = list_remove(scene->forcers, i);
        free(rem->bodies);
        free(rem);
        i--;
        break;
      }
    }
  }

  for (size_t i = 0; i < scene->num_bodies; i++) {
    Body *b = list_get(scene->bodies, i);

    if (body_is_removed(b)) {
      body_free(list_remove(scene->bodies, i));
      scene->num_bodies--;
      i--;
    }
  }
}
