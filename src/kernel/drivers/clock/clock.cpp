#include <kernel/drivers/clock/clock.hpp>
#include <kernel/asm.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/asm.hpp>

int ticks = 0;

extern "C" void clock_handler(void) {
	outb(PIC_IO_PIC1, PIC_EOI);
	ticks++;
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
