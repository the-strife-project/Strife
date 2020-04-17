#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <common/types.hpp>
#include <kernel/boot.hpp>

void memutils_init();
uint32_t getFreeMemory();
uint32_t getAllMemory();

#endif
