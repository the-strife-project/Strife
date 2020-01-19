#ifndef REGS_H
#define REGS_H

#include <common/types.h>

struct regs16_t {
	uint16_t di, si, bp, sp, bx, dx, cx, ax;
	uint16_t gs, fs, es, ds, eflags;
} __attribute__((packed));

void V86(uint8_t intnum, struct regs16_t *regs);
extern "C" void int32(uint8_t intnum, struct regs16_t *regs);

#endif
