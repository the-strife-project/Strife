#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <common/types.h>
#include <kernel/boot.h>
#include <kernel/paging/paging.h>

void memutils_init();
uint32_t getFreeMemory();
uint32_t getAllMemory();

#endif
