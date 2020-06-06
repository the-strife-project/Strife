#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <common/types.hpp>

enum {
	KP_EXAMPLE,
	KP_OUTOFMEM,
	KP_MULTIBOOT,
	KP_VBE2,
	KP_GRAPHICS,
	KP_PCI,
	KP_DMA_BUSMASTER,
	KP_NO_LBA,
	KP_CD_NOT_FOUND,
	KP_HDD_NOT_FOUND
};

void panic(uint32_t id);

#endif
