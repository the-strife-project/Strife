#include <kernel/memutils/memutils.h>

struct multiboot_info* __global_mbinfo;

void memutils_init(struct multiboot_info* mbinfo) {
	__global_mbinfo = mbinfo;
}

uint32_t getFreeMemory() {
	return (__global_mbinfo->mem_upper) - paging_getUsedPages()*4;
}
