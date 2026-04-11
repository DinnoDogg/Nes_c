#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/color.h"

#include "include/mapper.h"

#define NEW_MAPPER(NUM, MAPPER) \
    case NUM: \
        *read = &read_mapper_##MAPPER; \
        *write = &write_mapper_##MAPPER; \
        *free = &free_mapper_##MAPPER; \
        printf("Mapper %s\n", #MAPPER); \
        return new_mapper_##MAPPER(rom_data)

struct mapper_nrom {
    uint8_t* prg_rom;
    uint8_t* chr_rom;
    bool dual_bank : 1;
};

void* new_mapper(uint8_t* rom_data, bool* success, read_mapper* read, write_mapper* write, free_mapper* free) {
    uint8_t mapper_low = rom_data[0x6] >> 4, mapper_high = rom_data[0x7] >> 4;
    uint8_t mapper_number = (mapper_high << 4) | mapper_low;

    *success = true;

    switch (mapper_number) {
        NEW_MAPPER(0x00, nrom);
    }

    printf(BRED "Unsupported mapper, cannot boot.\n" COLOR_RESET);

    *success = false;

    *read = NULL;
    *write = NULL;
    *free = NULL;

    return NULL;
}

mapper_nrom_t* new_mapper_nrom(uint8_t* rom_data) {
    uint8_t prg_size = rom_data[0x4];

    mapper_nrom_t* result = (mapper_nrom_t*) malloc(sizeof(mapper_nrom_t));

    result->dual_bank = (prg_size == 0x2);
    result->prg_rom = rom_data + 0x10;  
    result->chr_rom = rom_data + 0x10 + (0x3FFF * prg_size);

    return result;
}

void free_mapper_nrom(void* mapper) {
    free(mapper);
}

uint8_t read_mapper_nrom(void* mapper, uint16_t address, uint8_t open_bus, mapper_access_source_t source) {
    mapper_nrom_t* mapper_ptr = (mapper_nrom_t*) mapper;

    if (source == SOURCE_PPU) {
        if (address < 0x2000) {
            return mapper_ptr->chr_rom[address];
        } 

        return open_bus;
    }

    if (address < 0x8000) {
        return open_bus;
    }

    if (address < 0xC000 || !mapper_ptr->dual_bank) {
        return mapper_ptr->prg_rom[address & 0x3FFF];
    }

    return mapper_ptr->prg_rom[address & 0x7FFE];
}

void write_mapper_nrom(void* mapper, uint16_t address, uint8_t data, mapper_access_source_t source) {
    return;
}

#undef NEW_MAPPER