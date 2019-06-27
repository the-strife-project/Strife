/*
	This is based almost completely on the following code:
		https://github.com/AlexandreRouma/LimeOS/blob/master/src/arch/i686/paging/paging.cpp
*/

#include <kernel/paging/paging.h>
#include <common/types.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <boot.h>
#include <kernel/asm.h>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024][1024] __attribute__((aligned(4096)));

uint32_t _maxmem = 0;	// Max memory in KiB.

void paging_enable() {
	// Init page tables.
	// Set all page tables to RW, and the physical page address.
	// Here I use uint32_t in order to not have overflow issues and such.
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			page_tables[i][j] = ((i * 0x400000) + (j * 0x1000)) | PT_RW;
		}
	}

	// Fill page directory.
	// Set all page directories to the address, and enable RW and present.
	for(uint16_t i=0; i<1024; i++)
		page_directory[i] = ((uint32_t)&page_tables[i][0]) | PD_RW | PD_PRESENT;

	// Allocate for the kernel.
	paging_setPresent(0, ((uint32_t)ASM_KERNEL_END / 4096) + 1);

	// Load page directory and enable.
	outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);
	go_paging(page_directory);
}

void paging_mapPage(uint32_t phy, uint32_t virt, uint16_t flags) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x03FF;	// 10 low bits of >> 4KiB.
	page_tables[pdi][pti] = phy | flags;
	invlpg(virt);
}

uint32_t paging_getPhysicalAddr(uint32_t virt) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x03FF;
	return page_tables[pdi][pti] & 0xFFFFF000;
}

uint16_t paging_getFlags(uint32_t virt) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x03FF;
	return page_tables[pdi][pti] & 0xFFF;
}
void paging_setFlags(uint32_t virt, uint32_t count, uint16_t flags) {
	uint32_t page_n = virt / 4096;
	for(uint32_t i=page_n; i<page_n+count; i++) {
		page_tables[i/1024][i%1024] &= 0xFFFFF000;	// Clear flags
		page_tables[i/1024][i%1024] |= flags;
		invlpg(i * 4096);
	}
}

uint16_t paging_getDirectoryFlags(uint32_t virt) {
	return page_directory[virt >> 22] & 0xFFF;
}
void paging_setDirectoryFlags(uint32_t virt, uint32_t count, uint16_t flags) {
	uint32_t pdi = virt >> 22;
    for (uint32_t i = pdi; i < pdi + count; i++) {
		page_directory[i] &= 0xFFFFF000;
		page_directory[i] |= flags;
	}
}

inline void paging_setPresent(uint32_t virt, uint32_t count) {
	uint32_t page_n = virt / 4096;
    for (uint32_t i = page_n; i < page_n + count; i++) {
        page_tables[i / 1024][i % 1024] |= PT_PRESENT;
        invlpg(i * 4096);
	}
}
inline void paging_setAbsent(uint32_t virt, uint32_t count) {
	uint32_t page_n = virt / 4096;
    for (uint32_t i = page_n; i < page_n + count; i++) {
        page_tables[i / 1024][i % 1024] &= 0xFFFFFFFE; // Clears first bit. Can't use PT_PRESENT because its not a uint32_t
        invlpg(i * 4096);
	}
}

uint32_t paging_findPages(uint32_t count) {
	uint32_t continous = 0;
	uint32_t startDir = 0;
	uint32_t startPage = 0;
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			if ((page_tables[i][j] & PT_PRESENT) == 0) {
                continous++;
			} else {
				continous = 0;
                startDir = i;
				startPage = j + 1;
			}
			if(continous == count) return (startDir * 0x400000) + (startPage * 0x1000);
		}
	}

	kernel_panic(1);	// Out of memory.
	return 0;
}

uint32_t paging_allocPages(uint32_t count) {
	uint32_t ptr = paging_findPages(count);
	paging_setPresent(ptr, count);
	return ptr;
}

uint32_t paging_getUsedPages() {
	uint32_t n = 0;
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			uint8_t flags = page_tables[i][j] & 0x01;
			if(flags == 1) n++;
		}
	}
	return n;
}

uint32_t paging_sizeToPages(uint32_t size) {
	uint32_t n = size / 4096;
    if((size % 4096) > 0) n++;
	return n;
}
