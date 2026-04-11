#ifndef BUS_H
#define BUS_H

#include <stdint.h>

typedef struct bus bus_t;

bus_t* new_bus();
void free_bus(bus_t* bus);

uint8_t read_bus(bus_t* bus, uint16_t address);
void write_bus(bus_t* bus, uint16_t address, uint8_t data);

#endif