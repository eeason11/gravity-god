#include "list.h"
#include <stdlib.h>
#include <assert.h>

#define GROWTH_FACTOR 2

struct list {
  void** elements;
  size_t size;
  int capacity;
  FreeFunc freer;
};

List *list_init(size_t initial_size, FreeFunc freer) {
  List *list = malloc(sizeof(List));
  list->elements = malloc(initial_size * sizeof(void*));
  list->size = 0;
  list->capacity = initial_size;
  list->freer = freer;

  return list;
}

void list_free(List *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->elements[i]);
    }
  }

  free(list->elements);
  free(list);
}

size_t list_size(List *list) {
  return list->size;
}

void *list_get(List *list, size_t index) {
  assert (index >= 0 && index < list->size);

  return list->elements[index];
}

void list_set(List *list, size_t index, void *value) {
  assert (index >= 0 && index < list->size);

  list->elements[index] = value;
}

void list_resize(List *list) {
  size_t init_capacity = list->capacity;
  void** bigger;
  if (list->capacity == 0) {
    bigger = malloc(sizeof(void*));
    list->capacity = 1;
  }
  else {
    bigger = malloc(GROWTH_FACTOR * list->capacity * sizeof(void*));
    list->capacity *= 2;
  }
  for (size_t i = 0; i < init_capacity; i++) {
    bigger[i] = list->elements[i];
  }
  free(list->elements);
  list->elements = bigger;
}

void list_add(List *list, void *value) {
  if (list->size >= list->capacity) {
    list_resize(list);
  }

  list->elements[list->size] = value;
  list->size++;
}

void list_add_front(List *list, void *value) {
  if (list->size >= list->capacity) {
    list_resize(list);
  }

  for (size_t i = list->size; i > 0; i--) {
    list->elements[i] = list->elements[i-1];
  }

  list->elements[0] = value;
  list->size++;
}

void *list_remove(List *list, size_t index) {
  assert(index >= 0 && index < list->size);

  void *element = list->elements[index];
  for (size_t i = index; i < list->size - 1; i++) {
    list->elements[i] = list->elements[i+1];
  }

  list->size--;

  return element;
}
