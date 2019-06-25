#ifndef PIC_H
#define PIC_H

#define PIC_IO_PIC1 0x20
#define PIC_IO_PIC2 0xA0
#define PIC_IRQ_SLAVE 2
#define PIC_IRQ0 0x20
#define PIC_EOI 0x20

void pic_set_mask(void);
void pic_enable_irq(int irq);
void pic_disable_irq(int irq);
void pic_init(void);

#endif
