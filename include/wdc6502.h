#ifndef WDC6502_H
#define WDC6502_H

#include <stdbool.h>

typedef struct WDC6502 WDC6502_t;
typedef struct bus bus_t;

WDC6502_t* new_wdc6502(bus_t* bus);
void free_wdc6502(WDC6502_t* cpu);

void wdc6502_print_state(WDC6502_t* cpu);
void wdc6502_execute_instruction(WDC6502_t* cpu);

void wdc6502_set_irq_line(WDC6502_t* cpu, bool value);
void wdc6502_set_nmi(WDC6502_t* cpu);

#endif
