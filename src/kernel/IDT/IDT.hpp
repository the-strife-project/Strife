#ifndef IDT_H
#define IDT_H

#include <common/types.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/asm.hpp>

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

extern struct IDT_entry IDT[256];

extern uint32_t* default_ISR_array;
extern uint32_t empty_ISR;
extern "C" void load_idt(struct IDT_ptr* idtptr);

typedef void (*ISR_delegate_t)(uint32_t topesp);

void idt_init(void);
void IDT_setPL(uint32_t intno, uint8_t pl);
void ISR_delegate(uint32_t intno, ISR_delegate_t delegate);

// Helpers.
PushadRegs* ISR_get_PushadRegs(uint32_t esp);
iretValues* ISR_get_iretValues(uint32_t esp);
uint32_t ISR_get_error(uint32_t esp);

#endif
