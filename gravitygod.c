#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "include/scene.h"
#include "include/body.h"
#include "include/polygon_helper.h"
#include "include/forces.h"
#include "include/color.h"
#include "include/vector.h"
#include "include/sdl_wrapper.h"

// Window constants
#define WINDOW_MAX ((Vector) {.x = 80.0, .y = 80.0})
#define OUTER_BOUND 10
#define SCALE 10

// Game constants
#define PLAYER_LOCATION ((Vector) {.x = 10, .y = 40})
#define PLAYER_WIDTH 15
#define PLAYER_HEIGHT 20
#define COLLIDER_WIDTH 9
#define COLLIDER_HEIGHT 20
#define PLAYER_SPEED 25
#define PLAYER_INIT_FALL_SPEED 15
#define ANIMATION_FRAMES 11
#define ANIMATION_SPEED .05 // low number corresponds to faster animation
#define ARIANA_MASS 100
#define KANYE_MASS 110
#define EMOJI_MASS 50
#define EMOJI_SIZE 3
#define EMOJI_INITIAL_X 10
#define EMOJI_HEIGHT_OFF_GROUND 4
#define OBSTACLE_WIDTH 5
#define OBSTACLE_HEIGHT 10
#define BACKGROUND_INTERVAL 1
#define SIDES_BACKGROUND_STARS 4
#define RADIUS_BACKGROUND_STARS 1
#define SLIMNESS_BACKGROUND_STARS 2.5
#define STAR_MASS 10
#define NUM_STARS 30
#define ENOUGH 30
#define HEAD_SIZE 350
#define FONT_SIZE 100

// Physics constants
#define G 3e-10
#define EARTH_MASS 7e14

// Misc. constants
#define BLACK ((RGBColor) {.r = 0, .g = 0, .b = 0})
#define WHITE ((RGBColor) {.r = 1, .g = 1, .b = 1})
#define STAR_COLOR ((RGBColor) {.r = 1, .g = 1, .b = 1})
#define BACKGROUND_COLOR ((RGBColor) {.r = 0.075, .g = 0.098, .b = 0.318})

// Enums
typedef enum gravity_direction{Up, Down} Gravity_Direction;
typedef enum body_kind{Ariana, Kanye, Gravity, Obstacle, Emoji, Star} Kind;

// Game state struct
typedef struct game_state {
  double OBJECT_INTERVAL;
  double EMOJI_FREQUENCY;
  double DOUBLE_FREQUENCY;
  double difficuly_factor;
  bool game_is_over;
  bool to_increment_score;
  bool level_just_increased;
  bool in_start_menu;
  Kind curr_player_type;
  Scene* scene;
} Game_State;

// Player struct
typedef struct player {
  Body *body;
  int score;
  int level;
} Player;

int random_int_between(int min, int max) {
  return (int)(rand() % (max - min) + min);
}

void obstacle_collision(Body *player, Body *Obstacle, Vector axis, void *aux) {
  Game_State *gs = (Game_State*)aux;
  gs->game_is_over = true;
}

void emoji_collision(Body *player, Body *emoji, Vector axis, void *aux) {
  Game_State *gs = (Game_State*)aux;
  body_remove(emoji);
  gs->to_increment_score = true;
}

void hit_ground(Body *wall, Body *player, Vector axis, void *aux) {
  Scene *scene = (Scene*)aux;

  body_remove(wall);
  Body *new_wall = body_init_with_info(body_get_shape(wall), EARTH_MASS, BACKGROUND_COLOR, (void*)Gravity, NULL);
  scene_add_body(scene, new_wall);

  scene_tick_delete_only(scene);
  body_tick(player, 0); // To reset the forces active on the body
  body_set_velocity(player, VEC_ZERO);
}

// Creates a gravitational force on the player in a given direction
void create_gravity(Scene *scene, Gravity_Direction direction) {
  Vector gravity_location = {.x = PLAYER_LOCATION.x, .y = 0};
  if (direction == Up) {
    gravity_location.y = WINDOW_MAX.y - 5;
  }
  else {
    gravity_location.y = -.5;
  }
  List *gravity_points = rectangle_points(gravity_location, 2, 2);
  Body *gravity_body = body_init_with_info(gravity_points, EARTH_MASS, TRANSPARENT, (void*)Gravity, NULL);
  scene_add_body(scene, gravity_body);

  // Add force creators
  Body *player;
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    if (body_get_mass(b) == KANYE_MASS || body_get_mass(b) == ARIANA_MASS) {
      player = b;
    }
  }
  create_newtonian_gravity(scene, G, gravity_body, player);
  create_collision(scene, gravity_body, player, hit_ground, scene, NULL);
}

void flip_gravity(Scene *scene) {
  bool player_found = false;
  bool gravity_existed = false;
  Body *player;

  for (size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    Kind type = Emoji;
    if (body_get_mass(b) == KANYE_MASS) {
      type = Kanye;
    }
    else if (body_get_mass(b) == ARIANA_MASS) {
      type = Ariana;
    }
    else if (body_get_mass(b) != EMOJI_MASS) {
      type = (Kind)body_get_info(b);
    }

    if (!gravity_existed && type == Gravity) {
      body_remove(b);

      // Gravity previously pulled player down
      if (body_get_centroid(b).y < WINDOW_MAX.y / 2) {
        create_gravity(scene, Up);
      }
      // Gravity previously pulled player up
      else {
        create_gravity(scene, Down);
      }

      gravity_existed = true;
    }
    else if (!player_found && (type == Ariana || type == Kanye)) {
      player = b;
      player_found = true;
    }

    if (player_found && gravity_existed) {
      break;
    }
  }

  scene_tick_delete_only(scene);
  body_tick(player, 0);

  Vector curr_velocity = body_get_velocity(player);
  if (curr_velocity.y < 0 || body_get_centroid(player).y < 10) {
    body_set_velocity(player, (Vector){.x = 0, .y = PLAYER_INIT_FALL_SPEED});
  }
  else if (curr_velocity.y > 0 || body_get_centroid(player).y > WINDOW_MAX.y - 10) {
    body_set_velocity(player, (Vector){.x = 0, .y = -PLAYER_INIT_FALL_SPEED});
  }

  /* Cause an assertion to fail if we tried flipping gravity
   * when no gravity had ever been created or if we didn't create a player */
  assert(gravity_existed && player_found);
}

char* concat(const char *a, const char *b) {
    char *result = malloc(strlen(a) + strlen(b) + 1);
    strcpy(result, a);
    strcat(result, b);
    return result;
}

Body *create_ariana(Vector location) {
  List *textures = list_init(ANIMATION_FRAMES * 2, free);

  for (int i = 0; i < ANIMATION_FRAMES; i++) {
    char index[3];
    sprintf(index, "%d", i);
    char *path = concat("images/ariana", index);
    char *image_path = concat(path, ".png");
    SDL_Texture *ariana_texture = sdl_load_image(image_path);
    list_add(textures, (void*)ariana_texture);
  }

  for (int i = 0; i < ANIMATION_FRAMES; i++) {
    char index[3];
    sprintf(index, "%d", i);
    char *path = concat("images/ariana", index);
    char *image_path = concat(path, "flipped.png");
    SDL_Texture *ariana_texture = sdl_load_image(image_path);
    list_add(textures, (void*)ariana_texture);
  }

  List *player_points = rectangle_points(location, COLLIDER_WIDTH, COLLIDER_HEIGHT);
  Body *player = body_init_with_info(player_points, ARIANA_MASS, BACKGROUND_COLOR, (void*)textures, NULL);
  return player;
}

Body *create_kanye(Vector location) {
  List *textures = list_init(ANIMATION_FRAMES * 2, free);

  for (int i = 0; i < ANIMATION_FRAMES; i++) {
    char index[3];
    sprintf(index, "%d", i);
    char *path = concat("images/kanye", index);
    char *image_path = concat(path, ".png");
    SDL_Texture *kanye_texture = sdl_load_image(image_path);
    list_add(textures, (void*)kanye_texture);
  }

  for (int i = 0; i < ANIMATION_FRAMES; i++) {
    char index[3];
    sprintf(index, "%d", i);
    char *path = concat("images/kanye", index);
    char *image_path = concat(path, "flipped.png");
    SDL_Texture *kanye_texture = sdl_load_image(image_path);
    list_add(textures, (void*)kanye_texture);
  }

  List *player_points = rectangle_points(location, COLLIDER_WIDTH, COLLIDER_HEIGHT);
  Body *player = body_init_with_info(player_points, KANYE_MASS, BACKGROUND_COLOR, (void*)textures, NULL);
  return player;
}

void create_stars(Scene *scene) {
  for (int i = 0; i < NUM_STARS; i++) {
    List *star_points = polygon_points(VEC_ZERO, SIDES_BACKGROUND_STARS, RADIUS_BACKGROUND_STARS, SLIMNESS_BACKGROUND_STARS);
    Body *star = body_init_with_info(star_points, STAR_MASS, STAR_COLOR, (void*)Star, NULL);
    body_set_centroid(star, (Vector){.x = (double)random_int_between(0, (int)WINDOW_MAX.x),
      .y = (double)random_int_between(0, (int)WINDOW_MAX.y)});
      Vector star_vel = (Vector){.x = -1 * PLAYER_SPEED, .y = 0};
    body_set_velocity(star, star_vel);
    scene_add_body(scene, star);
}
}


Player *player_init(Kind type) {
  Body *player_body;
  if (type == Ariana) {
    player_body = create_ariana(PLAYER_LOCATION);
  }
  else {
    player_body = create_kanye(PLAYER_LOCATION);
  }

  Player *player = malloc(sizeof(Player));
  player->body = player_body;
  player->score = 0;
  player->level = 0;
  return player;
}

Body *create_ring(Vector location) {
  char *image_path = "images/ring.png";
  SDL_Texture *ring_texture = sdl_load_image(image_path);
  List *ring_points = rectangle_points(location, EMOJI_SIZE, EMOJI_SIZE);
  Body *ring = body_init_with_info(ring_points, EMOJI_MASS, BACKGROUND_COLOR, (void*)ring_texture, NULL);
  return ring;
}

Body *create_peach(Vector location) {
  char *image_path = "images/peach.png";
  SDL_Texture *peach_texture = sdl_load_image(image_path);
  List *peach_points = rectangle_points(location, EMOJI_SIZE, EMOJI_SIZE);
  Body *peach = body_init_with_info(peach_points, EMOJI_MASS, BACKGROUND_COLOR, (void*)peach_texture, NULL);
  return peach;
}

void create_emoji(Player *player, Game_State *gs, int rand_seed) {
  Vector location = {.x = WINDOW_MAX.x - EMOJI_INITIAL_X, .y = 0};
  if (rand_seed % 2 == 0) {
    location.y = WINDOW_MAX.y - EMOJI_HEIGHT_OFF_GROUND;
  }
  else {
    location.y = EMOJI_HEIGHT_OFF_GROUND;
  }

  Body *emoji;
  if (body_get_mass(player->body) == ARIANA_MASS) {
    emoji = create_ring(location);
  }
  else {
    emoji = create_peach(location);
  }

  body_set_velocity(emoji, (Vector){.x = -PLAYER_SPEED, .y = 0});
  scene_add_body(gs->scene, emoji);
  create_collision(gs->scene, player->body, emoji, emoji_collision, gs, NULL);
}

void create_obstacle(Player *player, Game_State *gs, int rand_seed) {
  Vector location = {.x = WINDOW_MAX.x - EMOJI_INITIAL_X, .y = 0};
  if (rand_seed % 2 == 0) {
    location.y = WINDOW_MAX.y - OBSTACLE_HEIGHT / 2;
  }
  else {
    location.y = OBSTACLE_HEIGHT / 2;
  }

  List *points = rectangle_points(location, OBSTACLE_WIDTH, OBSTACLE_HEIGHT);
  Body *obstacle = body_init_with_info(points, INFINITY, BLACK, (void*)Obstacle, NULL);

  body_set_velocity(obstacle, (Vector){.x = -PLAYER_SPEED, .y = 0});
  scene_add_body(gs->scene, obstacle);
  create_collision(gs->scene, player->body, obstacle, obstacle_collision, gs, NULL);
}

void reset(Game_State *gs) {
gs->OBJECT_INTERVAL = 2.0;
gs->EMOJI_FREQUENCY = 0.3;
gs->DOUBLE_FREQUENCY = 0.3;
gs->difficuly_factor = 1.4;
gs->game_is_over = false;
gs->to_increment_score = false;
gs->level_just_increased = false;
}

void on_key(char key, KeyEventType type, double held_time, void *aux) {
  Scene *scene = (Scene*)aux;
  if (type == KEY_PRESSED) {
    switch(key) {
      case ' ':
        flip_gravity(scene);
        break;
    }
  }
}

void on_key_start_menu(char key, KeyEventType type, double held_time, void *aux) {
  Game_State *gs = (Game_State*)aux;
  if (type == KEY_PRESSED) {
    switch(key) {
      case ' ':
        gs->in_start_menu = false;
        break;
      case 'a':
        gs->curr_player_type = Ariana;
        break;
      case 'k':
        gs->curr_player_type = Kanye;
        break;
    }
  }
}

void on_key_endscreen(char key, KeyEventType type, double held_time, void *aux) {
  Game_State *gs = (Game_State*)aux;
  if (type == KEY_PRESSED) {
    switch(key) {
      case ' ':
        reset(gs);
        sdl_on_key(on_key, gs->scene);
        break;


    }
  }
}

void show_endscreen_text(Player *player) {
  char *made = "Made by: Ethan, Alison, Hannah, and Kellen";
  sdl_show_text(50, 700, 700, 100, 800, made, BLACK);

  char *play = "Press space to play again!";
  sdl_show_text(275, 425, 300, 100, 1000, play, BLACK);

  char *score = malloc(ENOUGH * sizeof(char));
  sprintf(score, "Score: %d", player->score);
  sdl_show_text(150, 100, 500, 200, 1000, score, BLACK);

  char *level = malloc(ENOUGH * sizeof(char));
  sprintf(level, "Level: %d", player->level);
  sdl_show_text(275, 300, 300, 100, 1000, level, BLACK);
}

void draw_emojis(Scene *scene) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    if (body_get_mass(b) == EMOJI_MASS) {
      SDL_Texture *texture = (SDL_Texture*)body_get_info(b);
      Vector centroid = body_get_centroid(b);
      Vector corner = {
        .x = (centroid.x - EMOJI_SIZE / 2),
        .y = (centroid.y - EMOJI_SIZE / 2)
      };
      // Flip over the center x axis
      if (corner.y < WINDOW_MAX.y / 2) {
        corner.y = WINDOW_MAX.y - EMOJI_HEIGHT_OFF_GROUND - EMOJI_SIZE * 2;
      }
      else {
        corner.y = EMOJI_HEIGHT_OFF_GROUND;
      }
      // Rescale
      corner.x *= SCALE;
      corner.y *= SCALE;

      sdl_draw_image(texture, corner, (Vector){.x = EMOJI_SIZE * SCALE * 2, .y = EMOJI_SIZE * SCALE * 2});
    }
  }
}

Vector get_corner(Vector centroid, double width, double height) {
  Vector corner = {
    .x = (centroid.x - width / 2),
    .y = (centroid.y - height / 2)
  };
  // Flip over the center x axis and shift
  corner.y = WINDOW_MAX.y - centroid.y - height / 2 - height / 6;
  corner.x += width / 6;

  // Rescale
  corner.x *= SCALE;
  corner.y *= SCALE;
  return corner;
}

void draw_players(Scene *scene, int animation) {
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    Body *b = scene_get_body(scene, i);
    if (body_get_mass(b) == ARIANA_MASS || body_get_mass(b) == KANYE_MASS) {
      List *textures = (List*)body_get_info(b);
      int index = animation;

      Vector centroid = body_get_centroid(b);
      Vector corner = get_corner(centroid, PLAYER_HEIGHT, PLAYER_WIDTH);
      if (centroid.y > WINDOW_MAX.y / 2) {
        index += ANIMATION_FRAMES;
      }
      SDL_Texture *texture = (SDL_Texture*)list_get(textures, index);
      sdl_draw_image(texture, corner, (Vector){.x = PLAYER_WIDTH * SCALE, .y = PLAYER_HEIGHT * SCALE});
    }
  }
}

void create_heads(Scene *scene) {
  char *path1 = "images/kanyeface.png";
  SDL_Texture *kanye_face_texture = sdl_load_image(path1);
  List *kanye_head_points = rectangle_points((Vector){.x = 15, .y = 18}, COLLIDER_WIDTH, COLLIDER_HEIGHT);
  Body *kanye_head = body_init_with_info(kanye_head_points, KANYE_MASS, WHITE, (void*)kanye_face_texture, NULL);
  scene_add_body(scene, kanye_head);

  char *path2 = "images/arianaface.png";
  SDL_Texture *ariana_face_texture = sdl_load_image(path2);
  List *ariana_head_points = rectangle_points((Vector){.x = 55, .y = 18}, COLLIDER_WIDTH, COLLIDER_HEIGHT);
  Body *ariana_head = body_init_with_info(ariana_head_points, KANYE_MASS, WHITE, (void*)ariana_face_texture, NULL);
  scene_add_body(scene, ariana_head);
}

void draw_heads(Scene *scene) {
  for (int i = 0; i < scene_bodies(scene); i++) {
    Body* curr_head = scene_get_body(scene, i);
    SDL_Texture *texture = (SDL_Texture*)body_get_info(curr_head);
    Vector corner = get_corner(body_get_centroid(curr_head), (HEAD_SIZE / SCALE), (HEAD_SIZE / SCALE));
    sdl_draw_image(texture, corner, (Vector){.x = HEAD_SIZE, .y = HEAD_SIZE});
  }
}

void show_startmenu_text() {
  char *kanye_option = "Press K to be Kanye West";
  sdl_show_text(50, 700, 300, 100, 1000, kanye_option, BLACK);

  char *ariana_option = "Press A to be Ariana Grande";
  sdl_show_text(450, 700, 300, 100, 1000, ariana_option, BLACK);

  char *to_start = "Press space to start!";
  sdl_show_text(275, 300, 300, 100, 1000, to_start, BLACK);

  char *title = "GRAVITY GOD";
  sdl_show_text(150, 100, 500, 200, 1000, title, BLACK);
}

SDL_Surface* new_score_level_surface(Player *p) {
  char *score = malloc(ENOUGH * sizeof(char));
  sprintf(score, "Score: %d", p->score);

  char *level = malloc(ENOUGH * sizeof(char));
  sprintf(level, "  Level: %d", p->level);

  char *combined = concat(score, level);
  TTF_Font *font = TTF_OpenFont("fonts/SF Atarian System.ttf", FONT_SIZE);
  SDL_Color White = {255, 255, 255};
  SDL_Surface* surface = TTF_RenderText_Blended(font, combined, White);
  return surface;
}

int main(int argc, char **argv) {
  // Initialize the scene, player, and game
  srand(time(NULL));
  sdl_init(VEC_ZERO, WINDOW_MAX);
  Scene *scene = scene_init();

  Game_State *gs = malloc(sizeof(Game_State));
  reset(gs);
  gs->scene = scene;
  gs->in_start_menu = true;
  gs->curr_player_type = Kanye;

  sdl_on_key(on_key_start_menu, gs);
  create_heads(scene);
  while (!sdl_is_done() && gs->in_start_menu) {
    sdl_clear();
    //scene_tick(scene, time_since_last_tick());
    show_startmenu_text();
    draw_heads(scene);
    sdl_show();
  }

  // Removes heads
  scene_remove_body(scene, 0);
  scene_remove_body(scene, 1);
  scene_tick(scene, time_since_last_tick());
  //sdl_quit();

  //sdl_init(VEC_ZERO, WINDOW_MAX);
  List *rectangle = rectangle_points((Vector){.x = WINDOW_MAX.x / 2, .y = WINDOW_MAX.y / 2}, WINDOW_MAX.x, WINDOW_MAX.y);
  Body *background = body_init(rectangle, 1, BACKGROUND_COLOR);
  scene_add_body(scene, background);
  create_stars(scene);

  Player *player = player_init(gs->curr_player_type);
  sdl_on_key(on_key, scene);
  scene_add_body(scene, player->body);
  create_gravity(scene, Down);

  SDL_Surface *surface = new_score_level_surface(player);

  double object_timer = 0;
  double background_timer = 0;
  double animation_timer = 0;
  int animation_frame = 0;
  int temp_score = 0;
  int newgame = 0;
  double dt = 0;

  while (!sdl_is_done()) {
    if (!gs->game_is_over) {
      if (newgame == 1){
        player->score = 0;
        player->level = 0;
        object_timer = 0;
        newgame = 0;
        dt = time_since_last_tick();
        for (int i = 0; i < scene_bodies(scene); i++) {
          Body *curr = scene_get_body(scene, i);
          if((Kind)body_get_info(curr) == Obstacle) {
                body_remove(curr);
          }
        }
        dt = time_since_last_tick();
        scene_tick(scene, dt);
      }
      sdl_clear();

  dt = time_since_last_tick();
      scene_tick(scene, dt);
      if (temp_score != player->score) {
        SDL_FreeSurface(surface);
        surface = new_score_level_surface(player);
        temp_score = player->score;
      }
      sdl_show_text_fast(500, 0, 200, 40, surface);
      if (animation_timer > ANIMATION_SPEED) {
        animation_frame++;
        if (animation_frame >= ANIMATION_FRAMES) {
          animation_frame = 0;
        }
        animation_timer = 0;
      }
      draw_players(scene, animation_frame);
      draw_emojis(scene);
      sdl_show();

      object_timer += dt;
      background_timer += dt;
      animation_timer += dt;

      if (object_timer > gs->OBJECT_INTERVAL) {
        int rand_seed = random_int_between(1, 100);
        if (rand_seed <= (gs->EMOJI_FREQUENCY + gs->DOUBLE_FREQUENCY) * 100) {
          create_emoji(player, gs, rand_seed);
        }
        if (rand_seed > gs->EMOJI_FREQUENCY * 100) {
          create_obstacle(player, gs, rand_seed + 1);
        }
        object_timer = 0;
      }
      if (gs->to_increment_score) {
        player->score++;
        gs->to_increment_score = false;
        if (gs->level_just_increased) {
          gs->level_just_increased = false;
        }
      }
      if (player->score % 10 == 0 && !(gs->level_just_increased)) {
        player->level++;
        gs->level_just_increased = true;
        gs->OBJECT_INTERVAL /= gs->difficuly_factor;
      }
      for (int i = 0; i < scene_bodies(scene); i++) {
        Body *curr = scene_get_body(scene, i);
        Vector curr_body_loc = body_get_centroid(curr);
        if (curr_body_loc.x < -OUTER_BOUND ||
            curr_body_loc.y < -OUTER_BOUND || curr_body_loc.y > WINDOW_MAX.y + OUTER_BOUND) {
          if ((Kind)body_get_info(curr) == Star) {
            body_set_centroid(curr, (Vector){.x = (double)random_int_between((int)WINDOW_MAX.x, 3/4 * (int)WINDOW_MAX.x),
              .y = (double)random_int_between(0, (int)WINDOW_MAX.y)});
          }
          else {
            if(body_get_mass(curr) != ARIANA_MASS && body_get_mass(curr) != KANYE_MASS) {
              body_remove(curr);
            }
          }
        }
      }
    }
    else {
      sdl_clear();
      show_endscreen_text(player);
      sdl_on_key(on_key_endscreen, gs);
      sdl_show();
      newgame = 1;
    }
  }
  scene_free(scene);
  free(gs);
  free(player);
  sdl_quit();
}
