#ifndef PPU_H
#define PPU_H

#include <stdint.h>

typedef struct WDC6502 WDC6502_t;
typedef struct bus bus_t;

typedef struct PPU PPU_t;

PPU_t* new_ppu(bus_t* bus, WDC6502_t* cpu);
void free_ppu(PPU_t* ppu);

uint8_t read_ppu(PPU_t* ppu, uint16_t address);
void write_ppu(PPU_t* ppu, uint16_t address, uint8_t data);

#endif