#ifndef IRQ_H
#define IRQ_H

#include <types.h>

struct irq_frame {
    uint32_t edi, esi, ebp, oesp, ebx, edx, ecx, eax;
    uint16_t pad1, fs, pad2, es, pad3, ds, pad4;

    uint32_t intno;
    uint32_t err;

    uint32_t eip;
    uint16_t cs, padding5;
    uint32_t eflags;

    uint32_t esp;
    uint16_t ss, padding6;
} __packed;

#endif
