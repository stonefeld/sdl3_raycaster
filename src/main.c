#include <stdio.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "utils.h"

#define WIDTH  800
#define HEIGHT 600

int main(int argc, char **argv) {
    WRAP_SDL_ERROR(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window;
    SDL_Renderer *renderer;

    char title[30];
    snprintf(title, sizeof(title), "Ray Casting - v%s", RAYCASTER_VERSION);
    WRAP_SDL_ERROR(SDL_CreateWindowAndRenderer(title, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer));

    unsigned char running = 1;
    SDL_Event event;

    float rect_size = 50.0f, x = (WIDTH - rect_size) / 2, y = (HEIGHT - rect_size) / 2;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    running = false;
                } break;

                case SDL_EVENT_MOUSE_MOTION: {
                    x = event.motion.x - rect_size / 2;
                    y = event.motion.y - rect_size / 2;
                } break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_FRect rect = {x, y, rect_size, rect_size};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
