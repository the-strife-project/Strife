#ifndef PAGING_H
#define PAGING_H

#include <common/types.hpp>

#define PAGE_SIZE 4096

#define PD_PRESENT  (1 << 0)
#define PD_RW       (1 << 1)
#define PD_ALL_PRIV (1 << 2)
#define PD_WRITETHR (1 << 3)
#define PD_CACHE_D  (1 << 4)
#define PD_ACCESSED (1 << 5)
#define PD_4M_PAGE  (1 << 7)

#define PT_PRESENT  (1 << 0)
#define PT_RW       (1 << 1)
#define PT_ALL_PRIV (1 << 2)
#define PT_WRITETHR (1 << 3)
#define PT_CACHE_D  (1 << 4)
#define PT_ACCESSED (1 << 5)
#define PT_DIRTY    (1 << 6)
#define PT_GLOBAL   (1 << 8)

void paging_init();
void paging_enable();
void paging_disable();

void paging_setPresent(uint32_t virt);
void paging_setUser(uint32_t virt);
uint32_t paging_allocPages(uint32_t count);
void paging_freePages(uint32_t virt, uint32_t count);

extern "C" void go_paging(uint32_t*);
extern "C" void switch_page_table(uint32_t*);
extern "C" void goback_paging();

extern uint32_t page_directory[1024];
extern uint32_t page_tables[1024][1024];

#endif
