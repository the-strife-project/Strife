#include <kernel/paging/paging.hpp>
#include <common/types.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/boot.hpp>
#include <kernel/asm.hpp>

// This is very inefficient. Improving it is not a priority.

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024][1024] __attribute__((aligned(4096)));

void paging_init() {
	// Init page tables.
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			page_tables[i][j] = ((j*0x1000) + (i*0x400000)) | PT_RW;
		}
	}

	// Fill page directory with page tables.
	for(uint32_t i=0; i<1024; i++) {
		page_directory[i] = ((uint32_t)page_tables[i]) | PD_RW | PD_PRESENT;
	}

	// Get available pages and mark them as present.
	paging_setPresent(0, getAllMemory() >> 2);	// Divided by 4 because it's in KiB.

	// Mark pages in use for the kernel (and everything below, such as VGA mappings).
	paging_setUsed(0, ((uint32_t)ASM_KERNEL_END >> 12)+1);
}

void paging_enable() { go_paging(page_directory); }
void paging_disable() { goback_paging(); }



void paging_mapPage(uint32_t phy, uint32_t virt, uint16_t flags) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = virt >> 12 & 0x03FF;	// 10 low bits of >> 4KiB.
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



void paging_setFlagUp(uint32_t virt, uint32_t count, uint32_t flag) {
	uint32_t page_n = virt / 4096;
	for(uint32_t i=page_n; i<page_n+count; i++) {
		page_tables[i / 1024][i % 1024] |= flag;
		invlpg(i * 4096);
	}
}

void paging_setFlagDown(uint32_t virt, uint32_t count, uint32_t flag) {
	uint32_t page_n = virt / 4096;
	for(uint32_t i=page_n; i<page_n+count; i++) {
		page_tables[i / 1024][i % 1024] &= ~flag;
		invlpg(i * 4096);
	}
}

void paging_setPresent(uint32_t virt, uint32_t count) {
	paging_setFlagUp(virt, count, PT_PRESENT);
}
void paging_setAbsent(uint32_t virt, uint32_t count) {
	paging_setFlagDown(virt, count, PT_PRESENT);
}
void paging_setUsed(uint32_t virt, uint32_t count) {
	paging_setFlagUp(virt, count, PT_USED);
}
void paging_setFree(uint32_t virt, uint32_t count) {
	paging_setFlagDown(virt, count, PT_USED);
}

void paging_setUser(uint32_t virt, uint32_t count) {
	uint32_t page_n = virt / 4096;
	for(uint32_t i=page_n; i<page_n+count; i+=1024) {
		page_directory[i/1024] |= PD_ALL_PRIV;
	}
	paging_setFlagUp(virt, count, PT_ALL_PRIV);
}



uint32_t paging_findPages(uint32_t count) {
	uint32_t continous = 0;
	uint32_t startDir = 0;
	uint32_t startPage = 0;
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			if(!(page_tables[i][j] & PT_PRESENT) || (page_tables[i][j] & PT_USED)) {
				continous = 0;
                startDir = i;
				startPage = j + 1;
			} else {
				if(++continous == count) return (startDir * 0x400000) + (startPage * 0x1000);
			}
		}
	}

	kernel_panic(1);	// Out of memory.
	return 0;
}

uint32_t paging_allocPages(uint32_t count) {
	uint32_t ptr = paging_findPages(count);
	paging_setUsed(ptr, count);
	return ptr;
}

uint32_t paging_getUsedPages() {
	uint32_t n = 0;
	for(uint32_t i=0; i<1024; i++) {
		for(uint32_t j=0; j<1024; j++) {
			uint8_t flags = page_tables[i][j] & PT_USED;
			if(flags == 1) n++;
		}
	}
	return n;
}
