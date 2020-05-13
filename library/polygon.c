#include "polygon.h"
#include "vector.h"
#include <math.h>
#include <stdlib.h>

double polygon_area(List *polygon) {
  // Has 2 or less vertices, so no area
  if (list_size(polygon) < 3) {
    return 0.0;
  }

  double area = 0.0;

  Vector *firstVertex = (Vector*)list_get(polygon, 0);
  Vector *secondVertex = (Vector*)list_get(polygon, 1);
  Vector *lastVertex = (Vector*)list_get(polygon, list_size(polygon) - 1);
  Vector *secondLastVertex = (Vector*)list_get(polygon, list_size(polygon) - 2);

  // Edge from the 1st vertex to the 2nd vertex
  area += firstVertex->x * (secondVertex->y - lastVertex->y);

  // Edge from the last vertex to the 1st vertex
  area += lastVertex->x * (firstVertex->y - secondLastVertex->y);

  // All other edges
  size_t i;
  for (i = 1; i < list_size(polygon) - 1; i++) {
    Vector *curr = (Vector*)list_get(polygon, i);
    Vector *next = (Vector*)list_get(polygon, i+1);
    Vector *prev = (Vector*)list_get(polygon, i-1);

    area += curr->x * (next->y - prev->y);
  }

  return area / 2;
}

Vector polygon_centroid(List *polygon) {
  double xSum = 0.0;
  double ySum = 0.0;

  size_t i;
  for (i = 0; i < list_size(polygon) - 1; i++) {
    Vector *curr = (Vector*)list_get(polygon, i);
    Vector *next = (Vector*)list_get(polygon, i+1);

    xSum += (curr->x + next->x) * vec_cross(*curr, *next);
    ySum += (curr->y + next->y) * vec_cross(*curr, *next);
  }

  // Edge from the 1st vertex to the last vertex
  Vector *first = (Vector*)list_get(polygon, 0);
  Vector *last = (Vector*)list_get(polygon, list_size(polygon) - 1);

  xSum += (last->x + first->x) * vec_cross(*last, *first);
  ySum += (last->y + first->y) * vec_cross(*last, *first);

  double area = polygon_area(polygon);
  Vector*center = malloc(sizeof(Vector));
  center->x = 1/(6*area) * xSum;
  center->y = 1/(6*area) * ySum;

  return *center;
}

void polygon_translate(List *polygon, Vector translation) {
  size_t i;
  for (i = 0; i < list_size(polygon); i++) {
    Vector newVec = vec_add(*((Vector*)list_get(polygon, i)), translation);
    Vector *newVecPointer = malloc(sizeof(Vector));
    newVecPointer->x = newVec.x;
    newVecPointer->y = newVec.y;
    list_set(polygon, i, (void*)newVecPointer);
  }
}

void polygon_rotate(List *polygon, double angle, Vector point) {
  polygon_translate(polygon, vec_negate(point));

  // Rotate the points around the origin
  size_t i;
  for (i = 0; i < list_size(polygon); i++) {
    Vector newVec = vec_rotate(*((Vector*)list_get(polygon, i)), angle);
    Vector*newPoint = malloc(sizeof(Vector));
    newPoint->x = newVec.x;
    newPoint->y = newVec.y;
    list_set(polygon, i, (void*)newPoint);
  }

  polygon_translate(polygon, point);
}