#include <kernel/TSS/TSS.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/paging/paging.hpp>

struct tss_entry_struct tss_entry;

// Thanks: https://wiki.osdev.org/TSS
void TSS_write(uint64_t* gdt) {
	// Make sure the TSS entry is zeroed.
	for(uint8_t i=0; i<sizeof(struct tss_entry_struct); i++) {
		((uint8_t*)&tss_entry)[i] = 0;
	}

	// Time to fill up the GDT entry.
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = base + sizeof(struct tss_entry_struct);
	gdt[_TSS_SEGMENT_N] = createGDTEntry(
		base,	// The address as the base.
		limit,	// The limit as the size.
		0xE9 |	// Present | Executable | Accessed
		SEG_SIZE(1)	// Size bit
	);

	// Set the kernel datasegment descriptor.
	tss_entry.ss0 = _KERNEL_DATASEGMENT;
	tss_entry.iomap_base = sizeof(struct tss_entry_struct);
}

void TSS_flush() {
	// Stack pointer at syscall. A page? Maybe more? I don't know.
	tss_entry.esp0 = PAGE_SIZE + kernelPaging.alloc();

	asm volatile(
		"ltr %%ax"
		: : "a" (_TSS_SEGMENT | 3)
	);
}
