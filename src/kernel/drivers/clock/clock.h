#ifndef CLOCK_H
#define CLOCK_H

#include <common/types.h>

#define CLOCK_IDT_ENTRY 0x20

extern void IDT_clock(void);
void clock_handler(void);

void clock_init(void);
void clock_start(void);
void clock_stop(void);
uint32_t clock_get(void);

#endif
