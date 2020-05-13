#include "body.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

void test_body_init() {
    Vector v[] = {{1, 1}, {2, 1}, {2, 2}, {1, 2}};
    const size_t VERTICES = sizeof(v) / sizeof(*v);
    List *shape = list_init(0, free);
    for (size_t i = 0; i < VERTICES; i++) {
        Vector *list_v = malloc(sizeof(*list_v));
        *list_v = v[i];
        list_add(shape, list_v);
    }
    RGBColor color = {0, 0.5, 1};
    Body *body = body_init(shape, 3, color);
    List *shape2 = body_get_shape(body);
    assert(list_size(shape2) == VERTICES);
    for (size_t i = 0; i < VERTICES; i++) {
        assert(vec_isclose(*(Vector *) list_get(shape2, i), v[i]));
    }
    list_free(shape2);
    assert(vec_isclose(body_get_centroid(body), (Vector) {1.5, 1.5}));
    assert(vec_equal(body_get_velocity(body), VEC_ZERO));
    assert(body_get_color(body).r == color.r);
    assert(body_get_color(body).g == color.g);
    assert(body_get_color(body).b == color.b);
    assert(body_get_mass(body) == 3);
    body_free(body);
}

void test_body_setters() {
    List *shape = list_init(3, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {+1, 0};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {0, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, 0};
    list_add(shape, v);
    Body *body = body_init(shape, 1, (RGBColor) {0, 0, 0});
    body_set_velocity(body, (Vector) {+5, -5});
    assert(vec_equal(body_get_velocity(body), (Vector) {+5, -5}));
    assert(vec_isclose(body_get_centroid(body), (Vector) {0, 1.0 / 3.0}));
    body_set_centroid(body, (Vector) {1, 2});
    assert(vec_isclose(body_get_centroid(body), (Vector) {1, 2}));
    shape = body_get_shape(body);
    assert(list_size(shape) == 3);
    assert(vec_isclose(*(Vector *) list_get(shape, 0), (Vector) {2, 5.0 / 3.0}));
    assert(vec_isclose(*(Vector *) list_get(shape, 1), (Vector) {1, 8.0 / 3.0}));
    assert(vec_isclose(*(Vector *) list_get(shape, 2), (Vector) {0, 5.0 / 3.0}));
    list_free(shape);
    body_set_rotation(body, M_PI / 2);
    assert(vec_isclose(body_get_centroid(body), (Vector) {1, 2}));
    shape = body_get_shape(body);
    assert(list_size(shape) == 3);
    assert(vec_isclose(*(Vector *) list_get(shape, 0), (Vector) {4.0 / 3.0, 3}));
    assert(vec_isclose(*(Vector *) list_get(shape, 1), (Vector) {1.0 / 3.0, 2}));
    assert(vec_isclose(*(Vector *) list_get(shape, 2), (Vector) {4.0 / 3.0, 1}));
    list_free(shape);
    body_set_centroid(body, (Vector) {3, 4});
    assert(vec_isclose(body_get_centroid(body), (Vector) {3, 4}));
    shape = body_get_shape(body);
    assert(list_size(shape) == 3);
    assert(vec_isclose(*(Vector *) list_get(shape, 0), (Vector) {10.0 / 3.0, 5}));
    assert(vec_isclose(*(Vector *) list_get(shape, 1), (Vector) {7.0 / 3.0, 4}));
    assert(vec_isclose(*(Vector *) list_get(shape, 2), (Vector) {10.0 / 3.0, 3}));
    list_free(shape);
    body_free(body);
}

void test_body_tick() {
    const Vector A = {1, 2};
    const double DT = 1e-6;
    const int STEPS = 1000000;
    List *shape = list_init(4, free);
    Vector *v = malloc(sizeof(*v));
    v->x = v->y = -1;
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    v->x = v->y = +1;
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, +1};
    list_add(shape, v);
    Body *body = body_init(shape, 1, (RGBColor) {0, 0, 0});

    // Apply constant acceleration and ensure position is (a / 2) * t ** 2
    for (int i = 0; i < STEPS; i++) {
        double t = i * DT;
        assert(vec_isclose(body_get_centroid(body), vec_multiply(t * t / 2, A)));
        body_set_velocity(body, vec_multiply(t + DT / 2, A));
        body_tick(body, DT);
    }
    double t = STEPS * DT;
    Vector new_x = vec_multiply(t * t / 2, A);
    shape = body_get_shape(body);
    assert(vec_isclose(*(Vector *) list_get(shape, 0), vec_add((Vector) {-1, -1}, new_x)));
    assert(vec_isclose(*(Vector *) list_get(shape, 1), vec_add((Vector) {+1, -1}, new_x)));
    assert(vec_isclose(*(Vector *) list_get(shape, 2), vec_add((Vector) {+1, +1}, new_x)));
    assert(vec_isclose(*(Vector *) list_get(shape, 3), vec_add((Vector) {-1, +1}, new_x)));
    list_free(shape);
    body_free(body);
}

void test_infinite_mass() {
    List *shape = list_init(10, free);
    Vector *v = malloc(sizeof(*v));
    *v = VEC_ZERO;
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, 0};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {0, +1};
    list_add(shape, v);
    Body *body = body_init(shape, INFINITY, (RGBColor) {0, 0, 0});
    body_set_velocity(body, (Vector) {2, 3});
    assert(body_get_mass(body) == INFINITY);
    body_add_force(body, (Vector) {1, 1});
    body_tick(body, 1.0);
    assert(vec_equal(body_get_velocity(body), (Vector) {2, 3}));
    assert(vec_isclose(body_get_centroid(body), (Vector) {2.5, 3.5}));
    body_free(body);
}

void test_forces() {
    const double MASS = 10;
    const double DT = 0.1;
    List *shape = list_init(3, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {+1, 0};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {0, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, 0};
    list_add(shape, v);
    Body *body = body_init(shape, MASS, (RGBColor) {0, 0, 0});
    body_set_centroid(body, VEC_ZERO);
    Vector old_velocity = {1, -2};
    body_set_velocity(body, old_velocity);
    body_add_force(body, (Vector) {MASS * 3, MASS * 4});
    body_add_impulse(body, (Vector) {MASS * 10, MASS * 5});
    body_add_force(body, (Vector) {MASS * 3, MASS * 4});
    body_tick(body, DT);
    Vector new_velocity =
        vec_add(old_velocity, (Vector) {10 + 6 * DT, 5 + 8 * DT});
    assert(vec_isclose(body_get_velocity(body), new_velocity));
    Vector new_centroid =
        vec_multiply(DT / 2.0, vec_add(old_velocity, new_velocity));
    assert(vec_isclose(body_get_centroid(body), new_centroid));
    body_tick(body, DT);
    assert(vec_isclose(
        body_get_centroid(body),
        vec_add(new_centroid, vec_multiply(DT, new_velocity))
    ));
    body_free(body);
}

void test_body_remove() {
    List *shape = list_init(3, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {+1, 0};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {0, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, 0};
    list_add(shape, v);
    Body *body = body_init(shape, 1, (RGBColor) {0, 0, 0});
    assert(!body_is_removed(body));
    body_remove(body);
    assert(body_is_removed(body));
    body_remove(body);
    assert(body_is_removed(body));
    body_free(body);
}

void test_body_info() {
    List *shape = list_init(3, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {+1, 0};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {0, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, 0};
    list_add(shape, v);
    int *info = malloc(sizeof(*info));
    *info = 123;
    Body *body = body_init_with_info(shape, 1, (RGBColor) {0, 0, 0}, info, NULL);
    assert(*(int *) body_get_info(body) == 123);
    body_free(body);
    free(info);
}

void test_body_info_freer() {
    List *shape = list_init(3, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {+1, 0};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {0, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, 0};
    list_add(shape, v);
    List *info = list_init(3, free);
    int *info_elem = malloc(sizeof(*info_elem));
    *info_elem = 10;
    list_add(info, info_elem);
    info_elem = malloc(sizeof(*info_elem));
    *info_elem = 20;
    list_add(info, info_elem);
    info_elem = malloc(sizeof(*info_elem));
    *info_elem = 30;
    list_add(info, info_elem);
    Body *body = body_init_with_info(
        shape, 1, (RGBColor) {0, 0, 0}, info, (FreeFunc) list_free
    );
    assert(*(int *) list_get(body_get_info(body), 0) == 10);
    assert(*(int *) list_get(body_get_info(body), 1) == 20);
    assert(*(int *) list_get(body_get_info(body), 2) == 30);
    body_free(body);
}

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_body_init)
    DO_TEST(test_body_setters)
    DO_TEST(test_body_tick)
    DO_TEST(test_infinite_mass)
    DO_TEST(test_forces)
    DO_TEST(test_body_remove)
    DO_TEST(test_body_info)
    DO_TEST(test_body_info_freer)

    puts("body_test PASS");
    return 0;
}
