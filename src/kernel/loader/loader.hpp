#ifndef LOADER_H
#define LOADER_H

#include <common/types.h>
#include <klibc/STL/list>

list<uint8_t*> loadELF(uint8_t* data);

#endif
