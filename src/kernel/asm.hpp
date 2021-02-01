#ifndef ASM_H
#define ASM_H

#include <common/types.hpp>

inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0": "=a" (ret) : "dN" (port));
	return ret;
}
inline uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile("inw %1, %0": "=a" (ret) : "dN" (port));
	return ret;
}
inline uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %1, %0": "=a" (ret) : "dN" (port));
	return ret;
}

inline void outb(uint16_t port, uint8_t value) {
	 asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}
inline void outw(uint16_t port, uint16_t value) {
	asm volatile("outw %1, %0" : : "dN" (port), "a" (value));
}
inline void outl(uint16_t port, uint32_t value) {
	asm volatile("outl %1, %0" : : "dN" (port), "a" (value));
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

// Ugly, right? Yeah, don't care.
inline void insl(uint16_t port, uint32_t* buffer, uint32_t cnt) {
	uint32_t* buff = buffer + cnt;
	while(buffer != buff)
		*(buffer++) = inl(port);
}

// Registers pushed by 'pushad', obviously.
struct PushadRegs {
	size_t edi, esi, ebp, esp;
	size_t ebx, edx, ecx, eax;
};

struct iretValues {
	size_t eip;
	uint16_t cs;
	size_t flags, esp;
	uint16_t ss;
};

inline void bochs_breakpoint() {
	outw(0x8A00,0x8A00);
	outw(0x8A00,0x08AE0);
}

inline uint32_t getCR3() {
	uint32_t ret;
	asm volatile(
		"mov %%cr3, %0"
		: "=r" (ret)
		:: "%eax");
	return ret;
}

inline uint32_t getCR2() {
	uint32_t ret;
	asm volatile(
		"mov %%cr2, %0"
		: "=r" (ret)
		:: "%eax");
	return ret;
}

#endif
