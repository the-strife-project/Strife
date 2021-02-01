#ifndef PIC_H
#define PIC_H

#define PIC_IO_PIC1 0x20
#define PIC_IO_PIC2 0xA0
#define PIC_IRQ_SLAVE 2
#define PIC_IRQ0 0x20
#define PIC_EOI 0x20

#include <kernel/asm.hpp>

void pic_set_mask(void);

inline void pic_refresh(void) {
	pic_set_mask();
	sti();
}

void pic_enable_irq(int irq);
void pic_disable_irq(int irq);
void pic_init();
void pic_finished_handling();

#endif
