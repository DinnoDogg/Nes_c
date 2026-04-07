#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "include/bus.h"
#include "include/wdc6502.h"

#include "include/color.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL; 
static SDL_Texture* texture = NULL;

static const int screen_w = 256;
static const int screen_h = 224;

static const char title[] = "Janky NES";

int main(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer(title, screen_w, screen_h, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return 1;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

    if (texture == NULL) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderer, screen_w, screen_h, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetWindowAspectRatio(window, 8.0f / 7.0f,  8.0f / 7.0f);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    
    bool running = true;
    SDL_Event event;

    int buffer_pitch;

    srand(time(NULL));

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        uint8_t* frame_buffer;

        SDL_LockTexture(texture, NULL, (void*) &frame_buffer, &buffer_pitch);

        for (int i = 0; i < screen_w * screen_h * 4; i += 4) {
            bool pixel = rand() % 2;
            frame_buffer[i + 3] = 0xFF;

            if (pixel) {
                frame_buffer[i] = rand();
                frame_buffer[i + 1] = rand();
                frame_buffer[i + 2] = rand();
                continue;
            }

            frame_buffer[i] = rand();
            frame_buffer[i + 1] = rand();
            frame_buffer[i + 2] = rand();
        }

       // printf("pitch: %u\n", buffer_pitch);

        SDL_UnlockTexture(texture);
        frame_buffer = NULL;
        
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }


    /*bus_t* test_bus = new_bus();
    WDC6502_t* test_cpu = new_wdc6502(test_bus);



    wdc6502_set_irq_line(test_cpu, true);
    //wdc6502_set_nmi(test_cpu);
    wdc6502_execute_instruction(test_cpu);
    wdc6502_print_state(test_cpu);
    wdc6502_execute_instruction(test_cpu);
    //wdc6502_execute_instruction(test_cpu);

    free_wdc6502(test_cpu);
    free_bus(test_bus);*/

    return 0;
}