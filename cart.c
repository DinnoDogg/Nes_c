#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

#include "include/cart.h"
#include "include/mapper.h"

struct cart {
    read_mapper read;
    write_mapper write;
    free_mapper free;

    void* mapper;
};

cart_t* new_cart(uint8_t* rom_data, bool* success) {
    cart_t* result = (cart_t*) malloc(sizeof(cart_t));
    result->mapper = new_mapper(rom_data, success, &result->read, &result->write, &result->free);

    if (! *success) {
        free(result);
        return NULL;
    }

    return result;
}

void free_cart(cart_t* cart) {
    cart->free(cart->mapper);
    free(cart);
}

uint8_t cpu_read_cart(cart_t* cart, uint16_t address, uint8_t open_bus) {
    return cart->read(cart->mapper, address, open_bus, SOURCE_CPU);
}

void cpu_write_cart(cart_t* cart, uint16_t address, uint8_t data) {
    cart->write(cart->mapper, address, data, SOURCE_CPU);
}

uint8_t ppu_read_cart(cart_t* cart, uint16_t address, uint8_t open_bus) {
    return cart->read(cart->mapper, address, open_bus, SOURCE_PPU);
}

void ppu_write_cart(cart_t* cart, uint16_t address, uint8_t data) {
    cart->write(cart->mapper, address, data, SOURCE_PPU);
}