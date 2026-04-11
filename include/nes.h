#ifndef NES_H
#define NES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct NES NES_t;

NES_t* new_nes(uint8_t* rom_data, bool* success);
void free_nes(NES_t* nes);

#endif