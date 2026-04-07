#ifndef PPU_H
#define PPU_H

#include <stdint.h>

typedef struct PPU PPU_t;

PPU_t* new_ppu();
void free_ppu(PPU_t* ppu);

uint8_t read_ppu(PPU_t* ppu, uint16_t address);
void write_ppu(PPU_t* ppu, uint16_t address, uint8_t data);

#endif