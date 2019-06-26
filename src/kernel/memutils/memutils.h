#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <common/types.h>
#include <boot.h>
#include <kernel/paging/paging.h>

void memutils_init(struct multiboot_info* mbinfo);
uint32_t getFreeMemory();

#endif
