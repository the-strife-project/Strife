#ifndef PAGING_H
#define PAGING_H

#include <common/types.h>

#define PD_PRESENT  1 << 0
#define PD_RW       1 << 1
#define PD_ALL_PRIV 1 << 2
#define PD_WRITETHR 1 << 3
#define PD_CACHE_D  1 << 4
#define PD_ACCESSED 1 << 5
#define PD_4M_PAGE  1 << 7

#define PT_PRESENT  1 << 0
#define PT_RW       1 << 1
#define PT_ALL_PRIV 1 << 2
#define PT_WRITETHR 1 << 3
#define PT_CACHE_D  1 << 4
#define PT_ACCESSED 1 << 5
#define PT_DIRTY    1 << 6
#define PT_GLOBAL   1 << 8
#define PT_USED     1 << 9

void paging_init();
void paging_enable();
void paging_disable();

void paging_mapPage(uint32_t phy, uint32_t virt, uint16_t flags);
uint32_t paging_getPhysicalAddr(uint32_t virt);
uint16_t paging_getFlags(uint32_t virt);
void paging_setFlags(uint32_t virt, uint32_t count, uint16_t flags);

void paging_setFlagUp(uint32_t virt, uint32_t count, uint32_t flag);
void paging_setFlagDown(uint32_t virt, uint32_t count, uint32_t flag);
void paging_setPresent(uint32_t virt, uint32_t count);
void paging_setAbsent(uint32_t virt, uint32_t count);
void paging_setUsed(uint32_t virt, uint32_t count);
void paging_setFree(uint32_t virt, uint32_t count);
void paging_setUser(uint32_t virt, uint32_t count);

uint32_t paging_findPages(uint32_t count);
uint32_t paging_allocPages(uint32_t count);
uint32_t paging_getUsedPages();

extern "C" void go_paging(uint32_t*);
extern "C" void goback_paging();

#endif
