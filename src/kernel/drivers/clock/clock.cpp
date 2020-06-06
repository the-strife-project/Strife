#include <kernel/drivers/clock/clock.hpp>
#include <kernel/asm.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/asm.hpp>

volatile uint32_t ticks = 0;

// Should be a list or something.
bool sleeping = false;
volatile uint32_t sleep_ticks = 0;

extern "C" void clock_handler(void) {
	outb(PIC_IO_PIC1, PIC_EOI);
	++ticks;
	++sleep_ticks;
}

void set_frequency(uint16_t hz) {
	uint16_t divisor = INPUT_CLOCK_FREQUENCY / hz;
	// Init, Square Wave Mode, non-BCD, first transfer LSB then MSB
	outb(TIMER_COMMAND, TIMER_ICW);
	outb(TIMER_DATA, divisor & 0xFF);
	outb(TIMER_DATA, (divisor >> 8) & 0xFF);
}

void clock_init(void) {
	set_frequency(1000);	// 1ms granularity
	IDT_SET_ENT(IDT[CLOCK_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_clock, 0);
}

void clock_start(void) {
	pic_enable_irq(0);
	ticks = 0;
}

void clock_stop(void) { pic_disable_irq(0); }

uint32_t clock_get(void) { return ticks; }

inline constexpr uint32_t msToTicks(uint32_t ms) {
	return ms+1;
}

void sleep(uint32_t ms) {
	sleep_ticks = 0;
	sleeping = true;

	while(sleep_ticks < msToTicks(ms)) {}

	sleeping = false;
	return;
}
