#include <kernel/drivers/clock/clock.h>
#include <klibc/stdio.h>
#include <kernel/asm.h>
#include <kernel/GDT/GDT.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/drivers/term/term.h>
#include <kernel/asm.h>

int ticks = 0;

void clock_handler(void) {
	outb(PIC_IO_PIC1, PIC_EOI);
	ticks++;

	if(ticks % 10 == 0) blinkCursor();
}

void clock_init(void) {
	IDT_SET_ENT(IDT[CLOCK_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_clock, 0);
}

void clock_start(void) {
	pic_enable_irq(0);
	ticks = 0;
}

void clock_stop(void) { pic_disable_irq(0); }

uint32_t clock_get(void) { return ticks; }
