#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/color.h"

#include "include/nes.h"

#include "include/cart.h"
#include "include/wdc6502.h"
#include "include/ppu.h"
#include "include/bus.h"

struct NES {
    WDC6502_t* cpu;
    bus_t* bus;
    PPU_t* ppu;
    cart_t* cart;
};

NES_t* new_nes(uint8_t* rom_data, bool* success) {
    NES_t* result = (NES_t*) malloc(sizeof(NES_t));

    result->cart = new_cart(rom_data, success);

    if (! *success) {
        printf(BRED "Failed to start emulator.\n" COLOR_RESET);
        free(result);
        return NULL;
    }

    result->bus = new_bus();
    result->cpu = new_wdc6502(result->bus);
    result->ppu = new_ppu(result->bus, result->cpu);
}