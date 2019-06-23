/*
	This is based almost completely on the following code:
		https://github.com/AlexandreRouma/MemeOS/blob/master/libs/kernel/src/paging.cpp
*/

#include <kernel/paging.h>
#include <common/types.h>
#include <kernel/kernel_panic.h>
#include <boot.h>

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024][1024] __attribute__((aligned(4096)));

uint32_t _maxmem = 0;

void paging_enable(uint32_t max) {
	_maxmem = max;

	// Init page tables.
	// Set all page tables to RW, and the physical page address.
	// Here I use uint32_t in order to not have overflow issues and such.
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			page_tables[i][j] = ((i * 0x400000) + (j * 0x1000)) | 2;
		}
	}

	// Fill page directory.
	// Set all page directories to the address, and enable RW and present.
	for(uint16_t i=0; i<1024; i++) page_directory[i] = ((uint32_t)&page_tables[i][0]) | 3;

	// Allocate for the kernel.
	paging_setPresent(0, ((uint32_t)ASM_KERNEL_END / 4096)+1);

	// Load page directory and enable.
	go_paging(page_directory);
}

void paging_mapPage(uint32_t phy, uint32_t virt, uint16_t flags) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x03FF;	// 10 low bits of >> 4KiB.
	page_tables[pdi][pti] = phy | flags;
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
void paging_setFlags(uint32_t virt, uint16_t flags) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x03FF;
	page_tables[pdi][pti] &= 0xFFFFF000;
	page_tables[pdi][pti] |= flags;
}

uint16_t paging_getDirectoryFlags(uint32_t virt) {
	uint32_t pdi = virt >> 22;
	return page_directory[pdi] & 0xFFF;
}
void paging_setDirectoryFlags(uint32_t virt, uint16_t flags) {
	uint32_t pdi = virt >> 22;
	page_directory[pdi] &= 0xFFFFF000;
	page_directory[pdi] |= flags;
}

void __local_setPresentOrAbsent(uint32_t virt, uint32_t count, uint8_t type) {
	// 0: absent.
	// 1: present.
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x03FF;

	uint32_t localCount = 0;
	for(uint32_t i=pdi; i<1024; i++) {
		for(uint32_t j=pti; j<1024; j++) {
			if(type)	// Set present
				page_tables[i][j] |= 1;
			else		// Set absent
				page_tables[i][j] &= 0xFFFFFFFE;
			localCount++;
			if(localCount >= count) return;
		}
	}
}
inline void paging_setPresent(uint32_t virt, uint32_t count) {
	__local_setPresentOrAbsent(virt, count, 1);
}
inline void paging_setAbsent(uint32_t virt, uint32_t count) {
	__local_setPresentOrAbsent(virt, count, 0);
}

uint32_t paging_findPages(uint32_t count) {
	uint32_t continous = 0;
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			uint8_t free = !(page_tables[i][j] & 1);
			if(free) continous++;
			else continous=0;

			if(continous == count) return (i*0x400000) + (j*0x1000);
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

uint32_t getUsedPages() {
	uint32_t n = 0;
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			uint8_t flags = page_tables[i][j] & 0x01;
			if(flags == 1) n++;
		}
	}
	return n;
}
