#include <kernel/paging/paging.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/boot.hpp>
#include <kernel/asm.hpp>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024][1024] __attribute__((aligned(4096)));

void paging_init() {
	// Fill page directory with page tables.
	for(uint32_t i=0; i<1024; ++i) {
		page_directory[i] = ((uint32_t)page_tables[i]);

		// Initialize all pages to its physical address. NOT PRESENT.
		for(uint32_t j=0; j<1024; ++j)
			page_tables[i][j] = i*0x400000 + j*0x1000;
	}

	// Kernel pages and everything below.
	uint32_t maxKernel = PAGE_SIZE + (uint32_t)ASM_KERNEL_END;
	for(uint32_t i=0; i<maxKernel; i+=PAGE_SIZE)
		paging_setPresent(i);

	// Possible memory mapped hardware.
	for(uint32_t i=0x00F00000; i<=0x00FFFFFF; i+=PAGE_SIZE)
		paging_setPresent(i);

	// Memory mapped devices.
	for(uint32_t i=0xC0000000; i; i+=PAGE_SIZE)
		paging_setPresent(i);
}

void paging_enable() { go_paging(page_directory); }
void paging_disable() { goback_paging(); }

// TODO: Optimize this bruteforce approach.
uint32_t paging_findPages(uint32_t count) {
	uint32_t continuous = 0;
	uint32_t sd = 0;
	uint32_t sp = 0;

	// TODO: this should be num_pd and num_pt instead of 1024. This will eventually break.
	for(uint16_t i=0; i<1024; ++i) {
		for(uint16_t j=0; j<1024; ++j) {
			if(page_tables[i][j] & PT_PRESENT) {
				// Not found yet.
				continuous = 0;
				sd = i;
				sp = j + 1;

				if(sp == 1024) {
					++sd;
					sp = 0;
				}
			} else {
				if(++continuous == count)
					return (sd * 0x400000) + (sp * 0x1000);
			}
		}
	}

	panic(KP_OUTOFMEM);
	return 0;
}

void paging_setPresent(uint32_t virt) {
	uint32_t page = virt / PAGE_SIZE;

	if(!(page_directory[page / 1024] & PD_PRESENT))
		page_directory[page / 1024] |= PD_PRESENT | PD_RW;

	page_tables[page / 1024][page % 1024] |= PT_PRESENT | PT_RW;
	invlpg(virt);
}

void paging_setUser(uint32_t virt) {
	uint32_t page = virt / PAGE_SIZE;
	page_directory[page / 1024] |= PD_ALL_PRIV;
	page_tables[page / 1024][page % 1024] |= PT_ALL_PRIV;
}

uint32_t paging_allocPages(uint32_t count) {
	uint32_t ptr = paging_findPages(count);

	uint32_t page_n = ptr / PAGE_SIZE;
	for(uint32_t i=page_n; i<page_n+count; ++i)
		paging_setPresent(i * PAGE_SIZE);

	return ptr;
}

void paging_freePages(uint32_t virt, uint32_t count) {
	// There's no point in freeing dirctionaries.

	uint32_t page_n = virt / PAGE_SIZE;
	for(uint32_t i=page_n; i<page_n+count; ++i) {
		page_tables[i / 1024][i % 1024] &= ~PT_PRESENT;
		invlpg(i * PAGE_SIZE);
	}
}
