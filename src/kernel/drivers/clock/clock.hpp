#ifndef CLOCK_H
#define CLOCK_H

#include <common/types.hpp>

#define CLOCK_IDT_ENTRY 0x20
#define INPUT_CLOCK_FREQUENCY 1193180
#define TIMER_COMMAND 0x43
#define TIMER_DATA 0x40
#define TIMER_ICW 0x36

void clock_handler(uint32_t esp);

void clock_init();
void clock_enable();
void clock_disable();
void sleep(uint32_t ms);

struct Timer {
	size_t id;
	size_t time;	// ms
	size_t count;	// ms passed since insertion
	bool repeat;	// Whether this timer is used repeatedly
};

size_t clock_addTimer(size_t time, bool repeat);

uint64_t clock_getMs();

#endif
