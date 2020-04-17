#ifndef CLOCK_H
#define CLOCK_H

#include <common/types.hpp>

#define CLOCK_IDT_ENTRY 0x20

extern "C" void IDT_clock(void);
extern "C" void clock_handler(void);

void clock_init(void);
void clock_start(void);
void clock_stop(void);
uint32_t clock_get(void);

#endif
