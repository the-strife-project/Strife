#include <GDT.h>

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
	uint64_t GDT[3];

	// First entry must be all zeros.
	GDT[0] = createGDTEntry(0, 0, 0);
	// Second entry: code segment.
	GDT[1] = createGDTEntry(0, 0x000FFFFF,
		SEG_DESCTYPE(1) |	// Code segment.
		SEG_PRES(1)	|	// Present.
		SEG_SAVL(0)	|	// Available to the system.
		SEG_LONG(0)	|
		SEG_SIZE(1)	|	// 32 bit protected mode.
		SEG_GRAN(1) |	// Use blocks of 4KiB.
		SEG_PRIV(0)	|	// Kernel privilege.
		SEG_CODE_EXRD	// Execute + Read
	);
	// Third entry: data segment (writable).
	GDT[2] = createGDTEntry(0, 0x000FFFFF,
		SEG_DESCTYPE(1) |	// Data segment.
		SEG_PRES(1)	|	// Present.
		SEG_SAVL(0)	|	// Available to the system.
		SEG_LONG(0)	|
		SEG_SIZE(1)	|	// 32 bit protected mode.
		SEG_GRAN(1) |	// Use blocks of 4KiB.
		SEG_PRIV(0)	|	// Kernel privilege.
		SEG_DATA_RDWR	// Read + Write
	);

	// Create the pointer, and load it.
	struct GDT_ptr gdtptr;
	gdtptr.size = sizeof(GDT);
	gdtptr.base = (uint32_t)GDT;
	load_gdt(&gdtptr);
}
