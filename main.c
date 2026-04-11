#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include "include/color.h"

#include "include/nes.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL; 
static SDL_Texture* texture = NULL;

static const int screen_w = 256;
static const int screen_h = 224;

static const char title[] = "Janky NES";

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(BRED "No ROM provided.\n" COLOR_RESET);
        return 1;
    }

    FILE* file = fopen(argv[1], "rb");
    
    if (file == NULL) {
        printf(BRED "Failed to open ROM.\n" COLOR_RESET);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);    
    uint8_t* rom_buffer = malloc(file_size);
    fread(rom_buffer, 1, file_size, file);

    NES_t* nes;
    bool success = true;

    nes = new_nes(rom_buffer, &success);

    if (!success) {
        free(rom_buffer);
        return 0;
    }

    return 0;
}