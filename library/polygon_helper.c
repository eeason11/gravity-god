#include "polygon_helper.h"
#include <stdlib.h>
#include <math.h>

List *polygon_points(Vector center, int vertices, int radius, double star_factor) {
    List *points = list_init(vertices * 2, free);

    double angle = M_PI / 2 * -1;
    int i = 0;
    for (i = 0; i < vertices * 2; i++) {
      angle += M_PI / vertices;

      double r = radius;
      if (i % 2 == 0) {
        r /= star_factor;
      }

      Vector* point = malloc(sizeof(Vector));
      point->x = r * cos(angle) + center.x;
      point->y = r * sin(angle) + center.y;

      list_add(points, point);
    }

    return points;
}

List *ellipse_points(Vector center, int vertices, int x_radius, int y_radius) {
    List *points = list_init(vertices, free);
    for (int i = vertices / 2; i >= -vertices / 2; i--) {
      Vector new = {.x = i, .y = y_radius * sqrt(1 - pow((double)i / (double)x_radius, 2))};
      Vector* new_point = malloc(sizeof(Vector));
      new_point->x = new.x;
      new_point->y = new.y;
      list_add(points, (void*)new_point);
    }
    for (int i = -vertices / 2; i <= vertices / 2; i++) {
      Vector new = {.x = i, .y = -(y_radius) * sqrt(1 - pow((double)i / (double)x_radius, 2))};
      Vector* new_point = malloc(sizeof(Vector));
      new_point->x = new.x;
      new_point->y = new.y;
      list_add(points, (void*)new_point);
    }
    polygon_translate(points, center);
    return points;
}

List* pie(Vector center, int vertices, int radius, double angle_start, double angle_end) {
    List *points = list_init(vertices * 2, free);
    double angle = angle_start;
    for (int i = 0; i < vertices; i++) {
      angle += (angle_end - angle_start) / vertices;

      Vector* point = malloc(sizeof(Vector));
      point->x = radius * cos(angle) + center.x;
      point->y = radius * sin(angle) + center.y;

      list_add(points, point);
    }

    return points;
}

List *rectangle_points(Vector center, double width, double height) {
  List *points = list_init(4, free);

  Vector *top_left = malloc(sizeof(Vector));
  top_left->x = center.x - width / 2;
  top_left->y = center.y + height / 2;

  Vector *bottom_left = malloc(sizeof(Vector));
  bottom_left->x = center.x - width / 2;
  bottom_left->y = center.y - height / 2;

  Vector *top_right = malloc(sizeof(Vector));
  top_right->x = center.x + width / 2;
  top_right->y = center.y + height / 2;

  Vector *bottom_right = malloc(sizeof(Vector));
  bottom_right->x = center.x + width / 2;
  bottom_right->y = center.y - height / 2;

  list_add(points, top_left);
  list_add(points, top_right);
  list_add(points, bottom_right);
  list_add(points, bottom_left);

  return points;
}
