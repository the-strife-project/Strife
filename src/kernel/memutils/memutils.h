#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <common/types.h>
#include <kernel/boot.h>

void memutils_init();
uint32_t getFreeMemory();
uint32_t getAllMemory();

#endif
