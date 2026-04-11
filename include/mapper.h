#ifndef MAPPER_H
#define MAPPER_H

#include <stdint.h>

typedef struct mapper_nrom mapper_nrom_t;

typedef enum mapper_access_source : short {
    SOURCE_CPU, 
    SOURCE_PPU
} mapper_access_source_t;

typedef uint8_t (*read_mapper)(void* mapper, uint16_t address, uint8_t open_bus, mapper_access_source_t source);
typedef void (*write_mapper)(void* mapper, uint16_t address, uint8_t data, mapper_access_source_t source);
typedef void (*free_mapper)(void* mapper);

void* new_mapper(uint8_t* rom_data, bool* success, read_mapper* read, write_mapper* write, free_mapper* free);

mapper_nrom_t* new_mapper_nrom(uint8_t* rom_data);
void free_mapper_nrom(void* mapper);
uint8_t read_mapper_nrom(void* mapper, uint16_t address, uint8_t open_bus, mapper_access_source_t source);
void write_mapper_nrom(void* mapper, uint16_t address, uint8_t data, mapper_access_source_t source);

#endif