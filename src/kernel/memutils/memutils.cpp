#include <kernel/memutils/memutils.hpp>
#include <kernel/paging/paging.hpp>

// Available RAM in KiB.
uint32_t memutils_memory;

struct memutils_entry {
	uint64_t base;
	uint64_t length;
	uint32_t type;
} __attribute__((packed));

struct memutils_entry* memutils_getEntries() {
	return (struct memutils_entry*)0xA000;
}

void memutils_init() {
	// Gets RAM memory from what JBoot left at 0xA000.
	// Basically, get the max base address and its length which is type 1 (free memory).
	// It's probably not meant to do it this way, but in the worst case scenario
	// it uses a little less memory than available, which is OK.
	uint64_t maxbase = 0;
	uint64_t maxlength = 0;
	for(struct memutils_entry* i=memutils_getEntries(); i->type; i++) {
		if(i->type == 1 && i->base > maxbase) {
			maxbase = i->base;
			maxlength = i->length;
		}
	}
	memutils_memory = maxbase + maxlength;
	memutils_memory /= 1024;
}

uint32_t getFreeMemory() {
	return memutils_memory - (paging_getUsedPages()*4);
}

uint32_t getAllMemory() { return memutils_memory; }
