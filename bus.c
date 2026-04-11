#include <stdint.h>
#include <stdlib.h>

#include "include/bus.h"

struct bus {
    uint8_t test_memory[0x10000];
    uint8_t open_bus;
};

bus_t* new_bus() {
    return (bus_t*) malloc(sizeof(bus_t));
}

void free_bus(bus_t* bus) {
    free(bus);
}

uint8_t read_bus(bus_t* bus, uint16_t address) {
    return bus->test_memory[address];
}

void write_bus(bus_t* bus, uint16_t address, uint8_t data) {
    bus->test_memory[address] = data;
}
