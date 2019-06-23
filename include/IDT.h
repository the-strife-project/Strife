#ifndef IDT_H
#define IDT_H

#include <types.h>
#include <IRQ.h>

struct IDT_entry {
	uint16_t base_low;
	uint16_t cs;
	uint8_t zero;
	uint8_t type :4;
	uint8_t s :1;
	uint8_t dpl :2;
	uint8_t p :1;
	uint16_t base_high;
} __attribute__((packed));

#define IDT_SET_ENT(ent, istrap, sel, off, d) do { \
	(ent).base_low = (uint32_t)(off) & 0xFFFF; \
	(ent).base_high = ((uint32_t)(off) >> 16) & 0xFFFF; \
	(ent).cs = sel; \
	(ent).zero = 0; \
	(ent).type = (istrap) ? 0xF : 0xE; \
	(ent).s = 0; \
	(ent).dpl = (d); \
	(ent).p = 1; \
} while (0)

struct IDT_ptr {
	uint16_t size;
	uint32_t base;
} __attribute__((packed));

struct IDT_entry IDT[256];

extern void unhandled_interrupt(void);
extern void load_idt(struct IDT_ptr* idtptr);

void idt_init(void);
void irq_global_handler(struct irq_frame *iframe);

#endif
