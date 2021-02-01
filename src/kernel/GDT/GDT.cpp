#include <kernel/GDT/GDT.hpp>
#include <kernel/TSS/TSS.hpp>

uint64_t GDT[GDT_ENTRIES];

uint64_t createGDTEntry(uint32_t base, uint32_t limit, uint16_t flags) {
	// See: https://wiki.osdev.org/images/f/f3/GDT_Entry.png
    uint64_t descriptor;

    // High 32 bit segment
    descriptor = limit & 0x000F0000;	// Size 19:16
    descriptor |= (flags << 8) & 0x00F0FF00;	// Access byte and flags
    descriptor |= (base >> 16) & 0x000000FF;	// Base 23:16
    descriptor |=  base & 0xFF000000;	// Base 31:24

    // Now the low 32 bit segment
    descriptor <<= 32;
    descriptor |= base << 16;	// Base 15:0
    descriptor |= limit & 0x0000FFFF;	// Size 15:0

    return descriptor;
}

void gdt_init(void) {
	// First entry must be all zeros.
	GDT[_GDT_NULL] = createGDTEntry(0, 0, 0);
	// Second entry: kernel code segment.
	GDT[_KERNEL_CODESEGMENT_N] = createGDTEntry(0, 0x000FFFFF,
		SEG_DESCTYPE(1) |	// Code segment.
		SEG_PRES(1)	|	// Present.
		SEG_SAVL(0)	|	// Available to the system.
		SEG_LONG(0)	|
		SEG_SIZE(1)	|	// 32 bit protected mode.
		SEG_GRAN(1) |	// Use blocks of 4KiB.
		SEG_PRIV(0)	|	// Kernel privilege.
		SEG_CODE_EXRD	// Execute + Read
	);
	// Third entry: kernel data segment (writable).
	GDT[_KERNEL_DATASEGMENT_N] = createGDTEntry(0, 0x000FFFFF,
		SEG_DESCTYPE(1) |	// Data segment.
		SEG_PRES(1)	|	// Present.
		SEG_SAVL(0)	|	// Available to the system.
		SEG_LONG(0)	|
		SEG_SIZE(1)	|	// 32 bit protected mode.
		SEG_GRAN(1) |	// Use blocks of 4KiB.
		SEG_PRIV(0)	|	// Kernel privilege.
		SEG_DATA_RDWR	// Read + Write
	);
	// Fourth entry: user code segment.
	GDT[_USER_CODESEGMENT_N] = createGDTEntry(0, 0x000FFFFF,
		SEG_DESCTYPE(1) |	// Code segment.
		SEG_PRES(1) |	// Present.
		SEG_SAVL(0) |	// Available to the system.
		SEG_LONG(0) |
		SEG_SIZE(1) |	// 32 bit protected mode.
		SEG_GRAN(1) |	// Blocks of 4KiB.
		SEG_PRIV(3) |	// User space.
		SEG_CODE_EXRD	// Execute + Read
	);
	// Fifth entry: user data segment.
	GDT[_USER_DATASEGMENT_N] = createGDTEntry(0, 0x000FFFFF,
		SEG_DESCTYPE(1) |	// Data segment.
		SEG_PRES(1) |	// Present.
		SEG_SAVL(0) |	// Available to the system.
		SEG_LONG(0) |
		SEG_SIZE(1) |	// 32 bit protected mode.
		SEG_GRAN(1) |	// Blocks of 4KiB.
		SEG_PRIV(3) |	// User space.
		SEG_DATA_RDWR	// Read + Write
	);
	// Sixth entry: TSS
	TSS_write(GDT);

	// Create the pointer and load it.
	struct GDT_ptr gdtptr;
	gdtptr.size = sizeof(GDT);
	gdtptr.base = (uint32_t)GDT;

	asm volatile("lgdt (%0)" : : "r" (&gdtptr));

	/*
		Reload CS and IP
		That '8' down there is _KERNEL_CODESEGMENT.
		I don't know how to make it use the constant.
		If you're reading this and you know, let me know.
	*/
	asm volatile(
		"jmpl $8, $1f\n"
		"1:\n"
		: : : "memory"
	);

	// Reset the segment registers to use the new GDT.
	asm volatile(
		"movw %w0, %%ss\n"
		"movw %w0, %%ds\n"
		"movw %w0, %%es\n"
		"movw %w0, %%fs\n"
		: : "r" (_KERNEL_DATASEGMENT)
		: "memory"
	);
}
