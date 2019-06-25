#ifndef ASM_H
#define ASM_H

#include <common/types.h>

inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0": "=a" (ret) : "dN" (port));
	return ret;
}

inline void outb(uint16_t port, uint8_t value) {
	 asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

inline void outw(uint16_t port, uint16_t value) {
	asm volatile("outw %1, %0" : : "dN" (port), "a" (value));
}

inline void cli(void) { asm volatile("cli"); }
inline void sti(void) { asm volatile ("sti"); }

inline uint32_t eflags_read(void) {
    uint32_t flags;
    asm volatile("pushfl\n"
                 "popl %0\n" : "=a" (flags));
    return flags;
}

inline void eflags_write(uint32_t flags) {
    asm volatile("pushl %0\n"
                 "popfl\n" : : "a" (flags));
}

inline void invlpg(uint32_t addr) {
	asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

#endif
