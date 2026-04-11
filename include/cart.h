#ifndef CART_H
#define CART_H

#include <stdint.h>
#include <stdbool.h>

typedef struct cart cart_t;

cart_t* new_cart(uint8_t* rom_data, bool* success);
void free_cart(cart_t* cart);

uint8_t cpu_read_cart(cart_t* cart, uint16_t address, uint8_t open_bus);
void cpu_write_cart(cart_t* cart, uint16_t address, uint8_t data);

uint8_t ppu_read_cart(cart_t* cart, uint16_t address, uint8_t open_bus);
void ppu_write_cart(cart_t* cart, uint16_t address, uint8_t data);

#endif