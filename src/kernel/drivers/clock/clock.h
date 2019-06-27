#ifndef CLOCK_H
#define CLOCK_H

#define CLOCK_IDT_ENTRY 0x20

extern void IDT_clock(void);
void clock_handler(void);

void clock_init(void);

#endif
