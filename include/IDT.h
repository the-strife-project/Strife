#ifndef IDT_H
#define IDT_H

#include <types.h>

struct IDT_entry {
	uint16_t off_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t off_high;
} __attribute__((packed));

struct IDT_ptr {
	uint16_t size;
	uint32_t base;
} __attribute__((packed));

extern void load_idt(struct IDT_ptr* idtptr);
extern void irq1(void);

void idt_init(void);

extern char read_port(char port);
extern void write_port(char port, char value);

#endif
