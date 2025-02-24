#include "SDL3/SDL_surface.h"
#define _USE_MATH_DEFINES
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
#include "SDL3_image/SDL_image.h"
#include "utils.h"

#define WIDTH      1280
#define HEIGHT     720
#define BLOCK_SIZE 10

#define OPACITY_DISTANCE 40

#define PLAYER_SIZE           20
#define PLAYER_MOVEMENT_SPEED 0.5
#define PLAYER_TURN_SPEED     0.03
#define PLAYER_PITCH          5
#define PLAYER_FOV            75
#define MAX_PITCH             200

struct player {
    float x, y, angle, size, pitch;
};

static struct player player = {
    .x = (float)WIDTH / 2,
    .y = (float)HEIGHT / 2,
    .angle = 0.01,
    .size = PLAYER_SIZE,
    .pitch = 0,
};
static char map[HEIGHT / BLOCK_SIZE][WIDTH / BLOCK_SIZE];
static float wall_height = HEIGHT << sizeof(float);

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

    return sqrtf((hit_x - cx) * (hit_x - cx) + (hit_y - cy) * (hit_y - cy));
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

    return sqrtf((hit_x - cx) * (hit_x - cx) + (hit_y - cy) * (hit_y - cy));
}

float draw_collision_lines(SDL_Renderer *renderer, float angle) {
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

    // get the collision distances
    float vertical = draw_vertical_collision_line(renderer, cx, cy, dir_x, dir_y, map_x, step_x);
    float horizontal = draw_horizontal_collision_line(renderer, cx, cy, dir_x, dir_y, map_y, step_y);

    // return the closest distance
    return vertical < horizontal ? vertical : horizontal;
}

void draw_walls(SDL_Renderer *renderer) {
    float deg_to_rad = M_PI / 180;
    float start = player.angle - PLAYER_FOV * deg_to_rad / 2;

    for (int i = 0; i < PLAYER_FOV; i++) {
        float angle = start + i * deg_to_rad;
        float distance = draw_collision_lines(renderer, angle);

        float point = distance * cosf(angle - player.angle);

        unsigned short r = OPACITY_DISTANCE * 228 / distance;
        unsigned short g = OPACITY_DISTANCE * 230 / distance;
        unsigned short b = OPACITY_DISTANCE * 168 / distance;

        SDL_SetRenderDrawColor(renderer, r > 228 ? 228 : r, g > 230 ? 230 : g, b > 168 ? 168 : b, 255);
        float height = wall_height / point;

        SDL_FRect rect = {
            .x = (float)i * WIDTH / PLAYER_FOV,
            .y = (HEIGHT - height) / 2 - (float)HEIGHT / 6 + player.pitch,
            .w = (float)WIDTH / PLAYER_FOV,
            .h = height,
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void draw_floor(SDL_Renderer *renderer) {
    int start_y = (float)HEIGHT / 2 - (float)HEIGHT / 6 + player.pitch;

    for (int i = start_y; i < HEIGHT; i += WIDTH / PLAYER_FOV) {
        int original_y = i - player.pitch;

        unsigned short r = OPACITY_DISTANCE * 2 / 3 * original_y / 198;
        unsigned short g = OPACITY_DISTANCE * 2 / 3 * original_y / 197;
        unsigned short b = OPACITY_DISTANCE * 2 / 3 * original_y / 139;

        SDL_SetRenderDrawColor(renderer, r > 198 ? 198 : r, g > 197 ? 197 : g, b > 139 ? 139 : b, 255);
        SDL_FRect rect = {0, (float)i, (float)WIDTH, (float)WIDTH / PLAYER_FOV};
        SDL_RenderFillRect(renderer, &rect);
    }
}

int main(int argc, char **argv) {
    WRAP_SDL_ERROR(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window;
    SDL_Renderer *renderer;

    char title[30];
    snprintf(title, sizeof(title), "Ray Casting - v%s", RAYCASTER_VERSION);
    WRAP_SDL_ERROR(SDL_CreateWindowAndRenderer(title, WIDTH, HEIGHT, 0, &window, &renderer));

    unsigned char relative_mouse = 1;
    WRAP_SDL_ERROR(SDL_SetWindowRelativeMouseMode(window, relative_mouse));

    unsigned char running = 1;
    SDL_Event event;

    unsigned char move_up = 0, move_down = 0, move_left = 0, move_right = 0, sprint = 1;
    unsigned char look_up = 0, look_down = 0, look_left = 0, look_right = 0;
    generate_map();

    SDL_Texture *sky_texture = IMG_LoadTexture(renderer, "assets/sky.png");
    SDL_SetTextureScaleMode(sky_texture, SDL_SCALEMODE_NEAREST);
    SDL_FRect sky_texture_rect = {0, 0, WIDTH, (float)sky_texture->h * WIDTH / sky_texture->w};

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

                        case SDLK_LSHIFT: sprint = 2; break;

                        case SDLK_UP: look_up = 1; break;
                        case SDLK_DOWN: look_down = 1; break;
                        case SDLK_LEFT: look_left = 1; break;
                        case SDLK_RIGHT: look_right = 1; break;
                    }
                } break;

                case SDL_EVENT_KEY_UP: {
                    switch (event.key.key) {
                        case SDLK_W: move_up = 0; break;
                        case SDLK_S: move_down = 0; break;
                        case SDLK_A: move_left = 0; break;
                        case SDLK_D: move_right = 0; break;

                        case SDLK_LSHIFT: sprint = 1; break;

                        case SDLK_UP: look_up = 0; break;
                        case SDLK_DOWN: look_down = 0; break;
                        case SDLK_LEFT: look_left = 0; break;
                        case SDLK_RIGHT: look_right = 0; break;

                        case SDLK_ESCAPE: {
                            relative_mouse = !relative_mouse;
                            SDL_SetWindowRelativeMouseMode(window, relative_mouse);
                        } break;
                    }
                } break;

                case SDL_EVENT_MOUSE_MOTION: {
                    if (relative_mouse) {
                        player.angle += event.motion.xrel * 0.005;
                        player.pitch -= event.motion.yrel;
                    }
                } break;
            }
        }

        float dx = cosf(player.angle), dy = sinf(player.angle);
        float front_speed = (move_up - move_down) * PLAYER_MOVEMENT_SPEED * sprint;
        float side_speed = (move_left - move_right) * PLAYER_MOVEMENT_SPEED * sprint;

        player.angle += (look_right - look_left) * PLAYER_TURN_SPEED;
        player.x += front_speed * dx + side_speed * dy;
        player.y += front_speed * dy - side_speed * dx;
        player.pitch += (look_up - look_down) * PLAYER_PITCH;

        // clamp the player's pitch
        if (player.pitch > MAX_PITCH)
            player.pitch = MAX_PITCH;
        if (player.pitch < -MAX_PITCH)
            player.pitch = -MAX_PITCH;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, sky_texture, NULL, &sky_texture_rect);
        SDL_RenderTextureTiled(renderer, sky_texture, NULL, 2, &sky_texture_rect);

        draw_floor(renderer);
        draw_walls(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(sky_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
