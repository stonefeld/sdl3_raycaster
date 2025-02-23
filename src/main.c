#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "utils.h"

#define WIDTH      800
#define HEIGHT     600
#define BLOCK_SIZE 50

#define PLAYER_SIZE       20
#define PLAYER_SPEED      3
#define PLAYER_TURN_SPEED 0.05

struct player {
    float x, y, angle, size;
};

static struct player player = {
    .x = (float)WIDTH / 2,
    .y = (float)HEIGHT / 2,
    .angle = 0.01,
    .size = PLAYER_SIZE,
};
static char map[HEIGHT / BLOCK_SIZE][WIDTH / BLOCK_SIZE];

void generate_map() {
    srand(time(NULL));

    for (int i = 0; i < HEIGHT / BLOCK_SIZE; i++) {
        for (int j = 0; j < WIDTH / BLOCK_SIZE; j++) {
            if (i == 0 || i == HEIGHT / BLOCK_SIZE - 1 || j == 0 || j == WIDTH / BLOCK_SIZE - 1)
                map[i][j] = 1;
            else
                map[i][j] = rand() % 5 == 0;
        }
    }
}

void draw_map(SDL_Renderer *renderer) {
    for (int i = 0; i < HEIGHT / BLOCK_SIZE; i++) {
        for (int j = 0; j < WIDTH / BLOCK_SIZE; j++) {
            if (map[i][j] == 1) {
                SDL_FRect rect = {j * BLOCK_SIZE + 1, i * BLOCK_SIZE + 1, BLOCK_SIZE - 1, BLOCK_SIZE - 1};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void draw_grid(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 80, 80, 255);
    for (int i = 0; i < HEIGHT / BLOCK_SIZE; i++)
        SDL_RenderLine(renderer, 0, i * BLOCK_SIZE, WIDTH, i * BLOCK_SIZE);
    for (int i = 0; i < WIDTH / BLOCK_SIZE; i++)
        SDL_RenderLine(renderer, i * BLOCK_SIZE, 0, i * BLOCK_SIZE, HEIGHT);
}

void draw_player(SDL_Renderer *renderer) {
    SDL_FRect rect = {player.x, player.y, player.size, player.size};
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

float draw_vertical_collision_line(SDL_Renderer *renderer, float cx, float cy, float dir_x, float dir_y, float map_x,
                                   int step_x) {
    // distance to the next vertical line
    float side_dist_x = ((dir_x > 0) ? ((map_x + 1) * BLOCK_SIZE - cx) : (cx - map_x * BLOCK_SIZE));
    float hipotenuse = fabs(side_dist_x / dir_x);
    float side_dist_y = dir_y * hipotenuse;

    float hit_x = cx + side_dist_x * step_x;
    float hit_y = cy + side_dist_y;

    hipotenuse = fabs(BLOCK_SIZE / dir_x);
    float increment_y = dir_y * hipotenuse;
    float increment_x = step_x * BLOCK_SIZE;

    while (hit_x >= 0 && hit_y >= 0 && hit_x < WIDTH && hit_y < HEIGHT
           && map[(int)hit_y / BLOCK_SIZE][(int)(hit_x + step_x) / BLOCK_SIZE] == 0) {
        hit_x += increment_x;
        hit_y += increment_y;
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderLine(renderer, cx, cy, hit_x, hit_y);

    return 0.0f;
}

float draw_horizontal_collision_line(SDL_Renderer *renderer, float cx, float cy, float dir_x, float dir_y, float map_y,
                                     int step_y) {
    // distance to the next horizontal line
    float side_dist_y = ((dir_y > 0) ? ((map_y + 1) * BLOCK_SIZE - cy) : (cy - map_y * BLOCK_SIZE));
    float hipotenuse = fabs(side_dist_y / dir_y);
    float side_dist_x = dir_x * hipotenuse;

    float hit_x = cx + side_dist_x;
    float hit_y = cy + side_dist_y * step_y;

    hipotenuse = fabs(BLOCK_SIZE / dir_y);
    float increment_x = dir_x * hipotenuse;
    float increment_y = step_y * BLOCK_SIZE;

    while (hit_x >= 0 && hit_y >= 0 && hit_x < WIDTH && hit_y < HEIGHT
           && map[(int)(hit_y + step_y) / BLOCK_SIZE][(int)hit_x / BLOCK_SIZE] == 0) {
        hit_x += increment_x;
        hit_y += increment_y;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderLine(renderer, cx, cy, hit_x, hit_y);

    return 0.0f;
}

void draw_collision_lines(SDL_Renderer *renderer, float angle) {
    float cx = player.x + player.size / 2;
    float cy = player.y + player.size / 2;

    // get the player directions in x and y coords
    float dir_x = cosf(angle) + 0.0001f;
    float dir_y = sinf(angle) + 0.0001f;

    // get the player position in the map
    int map_x = (int)cx / BLOCK_SIZE;
    int map_y = (int)cy / BLOCK_SIZE;

    // get the step according to the direction
    int step_x = dir_x > 0 ? 1 : -1;
    int step_y = dir_y > 0 ? 1 : -1;

    // draw the vertical and horizontal collision lines
    draw_vertical_collision_line(renderer, cx, cy, dir_x, dir_y, map_x, step_x);
    draw_horizontal_collision_line(renderer, cx, cy, dir_x, dir_y, map_y, step_y);
}

int main(int argc, char **argv) {
    WRAP_SDL_ERROR(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window;
    SDL_Renderer *renderer;

    char title[30];
    snprintf(title, sizeof(title), "Ray Casting - v%s", RAYCASTER_VERSION);
    WRAP_SDL_ERROR(SDL_CreateWindowAndRenderer(title, WIDTH, HEIGHT, 0, &window, &renderer));

    unsigned char running = 1;
    SDL_Event event;

    unsigned char move_up = 0, move_down = 0, move_left = 0, move_right = 0;
    generate_map();

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    running = false;
                } break;

                case SDL_EVENT_KEY_DOWN: {
                    switch (event.key.key) {
                        case SDLK_W: move_up = 1; break;
                        case SDLK_S: move_down = 1; break;
                        case SDLK_A: move_left = 1; break;
                        case SDLK_D: move_right = 1; break;
                    }
                } break;

                case SDL_EVENT_KEY_UP: {
                    switch (event.key.key) {
                        case SDLK_W: move_up = 0; break;
                        case SDLK_S: move_down = 0; break;
                        case SDLK_A: move_left = 0; break;
                        case SDLK_D: move_right = 0; break;
                    }
                } break;
            }
        }

        player.angle += (move_right - move_left) * PLAYER_TURN_SPEED;
        player.x += (move_up - move_down) * PLAYER_SPEED * cosf(player.angle);
        player.y += (move_up - move_down) * PLAYER_SPEED * sinf(player.angle);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_grid(renderer);
        draw_map(renderer);
        for (float angle = player.angle - 3.14 / 6; angle < player.angle + 3.14 / 6; angle += 0.01)
            draw_collision_lines(renderer, angle);
        draw_player(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
