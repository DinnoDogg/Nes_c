#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "include/ppu.h"

#define OAM_ADDRESS ppu->registers.OAMADDR

typedef struct Wx2_reg {
    union {
        uint8_t low;
        uint8_t high;

        uint16_t word;
    };

    bool access_low : 1;
} Wx2_reg_t;

struct PPU {
    struct {
        uint8_t PPUCTRL;
        uint8_t PPUMASK;
        uint8_t PPUSTATUS;
        uint8_t OAMADDR;

        Wx2_reg_t PPUSCROLL;
        Wx2_reg_t PPUADDR;
    } registers;

    uint8_t OAM[0x100];

    uint8_t open_bus;
};

PPU_t* new_ppu(bus_t* bus, WDC6502_t* cpu) {
    return NULL;
}

uint8_t read_ppu(PPU_t* ppu, uint16_t address) {
    uint8_t result = ppu->open_bus;

    /*switch (address) {
        case 0x2002: 
            result = ppu->registers.PPUSTATUS; 
            break;

        case 0x2002: 
            result = ppu->OAM[OAM_ADDRESS]; 
            break;

            
    }*/

    ppu->open_bus = result;
    return result;
}

void write_ppu(PPU_t* ppu, uint16_t address, uint8_t data);

#undef OAM_ADDRESS