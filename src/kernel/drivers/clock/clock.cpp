#include <kernel/drivers/clock/clock.hpp>
#include <kernel/asm.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/asm.hpp>
#include <dlist>
#include <kernel/tasks/scheduler/scheduler.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/TSS/TSS.hpp>

// This is not suitable for multitasking at all.
dlist<Timer> timers;
size_t nextID = 1;

// TODO: DON'T DO THIS. Use RTC instead, so there's no need for an interrupt every ms.
uint64_t globalMs = 0;	// Milliseconds since clock initialization.

#include <kernel/klibc/stdio>

void clock_handler(uint32_t esp) {
	// There might be a stack overwrite issue here. Not sure.
	pic_finished_handling();
	++globalMs;

	iretValues* iret = ISR_get_iretValues(esp);
	if(iret->cs == _KERNEL_CODESEGMENT) {
		// Coming from the kernel. Just return.
		return;
	} else {
		// Coming from usermode.
		PushadRegs* regs = ISR_get_PushadRegs(esp);
		tasks[schedulers[0].getCurrent()].task.update(regs, iret);
	}

	// Update timers, call cycle() if necessary.
	auto it = timers.begin();
	bool cyclePerformed = false;
	while(it != timers.end()) {
		auto& x = *it;
		if(++x.count == x.time) {
			cyclePerformed = true;
			if(x.repeat) {
				x.count = 0;
				++it;
			} else {
				timers.remove(it++);
			}
			schedulers[0].cycle(x.id);
		} else {
			++it;
		}
	}

	if(cyclePerformed) {
		schedulers[0].resume();
		printf(" {{ jlxip's fault: resume() returned?????? }} ");
		while(true) {}
	}
}

size_t clock_addTimer(size_t time, bool repeat) {
	Timer timer;
	timer.id = nextID++;
	timer.time = time;
	timer.count = 0;
	timer.repeat = repeat;

	clock_disable();
	timers.push_back(timer);
	clock_enable();

	return timer.id;
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
	//IDT_SET_ENT(IDT[CLOCK_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_clock, 3);
	ISR_delegate(CLOCK_IDT_ENTRY, clock_handler);
	timers = dlist<Timer>();
}

void clock_enable() {
	pic_enable_irq(0);
}

void clock_disable() {
	pic_disable_irq(0);
}

// TODO: Don't do this (?). Will probably become useless in some time.
void sleep(uint32_t ms) {
	auto current = globalMs;
	while(globalMs < current+1+ms);
	return;
}

uint64_t clock_getMs() {
	return globalMs;
}
