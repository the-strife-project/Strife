#ifndef CLOCK_H
#define CLOCK_H

#include <common/types.hpp>

#define CLOCK_IDT_ENTRY 0x20
#define INPUT_CLOCK_FREQUENCY 1193180
#define TIMER_COMMAND 0x43
#define TIMER_DATA 0x40
#define TIMER_ICW 0x36

extern "C" void IDT_clock(void);
extern "C" void clock_handler(void);

void clock_init(void);
void clock_start(void);
void clock_stop(void);
uint32_t clock_get(void);
void sleep(uint32_t ms);

#endif
