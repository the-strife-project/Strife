#ifndef REGS_H
#define REGS_H

struct regs16_t {
	uint16_t di, si, bp, sp, bx, dx, cx, ax;
	uint16_t gs, fs, es, ds, eflags;
} __attribute__((packed));

extern void int32(uint8_t intnum, struct regs16_t *regs);

#endif
