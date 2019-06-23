#ifndef PAGING_H
#define PAGING_H

#include <common/types.h>

void paging_enable(uint32_t max);
void paging_mapPage(uint32_t phy, uint32_t virt, uint16_t flags);
uint32_t paging_getPhysicalAddr(uint32_t virt);
uint16_t paging_getFlags(uint32_t virt);
void paging_setFlags(uint32_t virt, uint16_t flags);
uint16_t paging_getDirectoryFlags(uint32_t virt);
void paging_setDirectoryFlags(uint32_t virt, uint16_t flags);
void paging_setPresent(uint32_t virt, uint32_t count);
void paging_setAbsent(uint32_t virt, uint32_t count);
uint32_t paging_findPages(uint32_t count);
uint32_t paging_allocPages(uint32_t count);
uint32_t paging_getUsedPages();

extern void go_paging(uint32_t*);

#endif
