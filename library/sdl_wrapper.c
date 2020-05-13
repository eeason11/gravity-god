#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <time.h>
#include "sdl_wrapper.h"

#define WINDOW_TITLE "CS 3"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define MS_PER_S 1e3
#define ENOUGH 100000

/**
 * The coordinate at the center of the screen.
 */
Vector center;
/**
 * The coordinate difference from the center to the top right corner.
 */
Vector max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
KeyHandler key_handler = NULL;
void *aux = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return LEFT_ARROW;
        case SDLK_UP: return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN: return DOWN_ARROW;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

void sdl_init(Vector min, Vector max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);

    center = vec_multiply(0.5, vec_add(min, max)),
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
}

void sdl_quit(void) {
  IMG_Quit();
}

bool sdl_is_done(void) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                free(event);
                return true;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (!key_handler) break;
                char key = get_keycode(event->key.keysym.sym);
                if (!key) break;

                double timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time =
                    (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, type, held_time, aux);
                break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(List *points, RGBColor color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    if (color.r != -1 || color.g != -1 || color.b != -1) {
      assert(0 <= color.r && color.r <= 1);
      assert(0 <= color.g && color.g <= 1);
      assert(0 <= color.b && color.b <= 1);
    }

    // Scale scene so it fits entirely in the window,
    // with the center of the scene at the center of the window
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width);
    assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    free(width);
    free(height);
    double x_scale = center_x / max_diff.x,
           y_scale = center_y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;

    // Convert each vertex to a point on screen
    short *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points);
    assert(y_points);
    for (size_t i = 0; i < n; i++) {
        Vector *vertex = list_get(points, i);
        Vector pos_from_center =
            vec_multiply(scale, vec_subtract(*vertex, center));
        // Flip y axis since positive y is down on the screen
        x_points[i] = round(center_x + pos_from_center.x);
        y_points[i] = round(center_y - pos_from_center.y);
    }

    // Draw polygon with the given color
    if (color.r == -1 && color.g == -1 && color.b == -1) {
      filledPolygonRGBA(
          renderer,
          x_points, y_points, n,
          0, 0, 0, 0
      );
    }
    else {
      filledPolygonRGBA(
          renderer,
          x_points, y_points, n,
          color.r * 255, color.g * 255, color.b * 255, 255
      );
    }
    free(x_points);
    free(y_points);
}

void sdl_draw_pie(Vector pos, int rad, int start, int end, RGBColor color) {
    // Check parameters: none?

    // Scale scene so it fits entirely in the window,
    // with the center of the scene at the center of the window
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width);
    assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    free(width);
    free(height);
    double x_scale = center_x / max_diff.x,
           y_scale = center_y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;

    // Convert center position to a point on screen
    Vector pos_from_center =
            vec_multiply(scale, vec_subtract(pos, center));
    // Flip y axis since positive y is down on the screen
    int x = round(center_x + pos_from_center.x);
    int y = round(center_y - pos_from_center.y);

    // Draw polygon with the given color
    filledPieRGBA(
        renderer,
        x, y, rad,
        start, end, //angles
        color.r * 255, color.g * 255, color.b * 255, 255
    );
}

void sdl_show(void) {
    SDL_RenderPresent(renderer);
}

void sdl_render_scene(Scene *scene) {
    sdl_clear();
    size_t body_count = scene_bodies(scene);
    for (size_t i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        List *shape = body_get_shape(body);
        sdl_draw_polygon(shape, body_get_color(body));
        list_free(shape);
    }
    sdl_show();
}

void sdl_on_key(KeyHandler handler, void *helper) {
    key_handler = handler;
    aux = helper;
}

void sdl_show_text(int x, int y, int width, int height, int font_size, char *str, RGBColor color) {

	TTF_Font *font = TTF_OpenFont("fonts/SF Atarian System.ttf", font_size);
  SDL_Color Black = {color.r * 255, color.g * 255, color.b * 255};

  SDL_Surface* str_surface = TTF_RenderText_Blended(font, str, Black);
  SDL_Texture* str_texture = SDL_CreateTextureFromSurface(renderer, str_surface);
  SDL_Rect str_rect = {.x = x, .y = y, .w = width, .h = height};
  SDL_RenderCopy(renderer, str_texture, NULL, &str_rect);

  SDL_DestroyTexture(str_texture);
  SDL_FreeSurface(str_surface);
  TTF_CloseFont(font);
}

void sdl_show_text_fast(int x, int y, int width, int height, SDL_Surface* surface) {
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect str_rect = {.x = x, .y = y, .w = width, .h = height};
  SDL_RenderCopy(renderer, texture, NULL, &str_rect);
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

SDL_Texture *sdl_load_image(char *path) {
  SDL_Surface *image;
  image = IMG_LoadTyped_RW(SDL_RWFromFile(path, "rb"), 1, "PNG");

  if (!image) {
    printf("IMG_Load error: %s\n", IMG_GetError());
    printf("path: %s\n", path);
    return NULL;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
  return texture;
}

void sdl_draw_image(SDL_Texture *texture, Vector location, Vector size) {
  SDL_Rect loc_rec = {location.x, location.y, size.x, size.y};
  SDL_RenderCopy(renderer, texture, NULL, &loc_rec);
}
