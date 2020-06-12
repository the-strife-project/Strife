#include <kernel/tasks/task.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/klibc/stdio>

Task createTask(PID_t pid, uint32_t priority, PID_t parent, const list<pair<uint32_t, uint32_t>>& pages) {
	Task ret;
	ret.PID = pid;
	ret.priority = priority;
	ret.parent = parent;

	// Prepare page table.
	uint32_t* pd = (uint32_t*)paging_allocPages(1);
	ret.pageDirectory = pd;

	// Copy kernel's page table.
	for(uint16_t pdi=0; pdi<1024; ++pdi) {
		if(!(page_directory[pdi] & PD_PRESENT))
		  continue;

		uint32_t* pt = (uint32_t*)paging_allocPages(1);
		pd[pdi] = (uint32_t)pt | PD_RW | PD_PRESENT;

		for(uint16_t pti=0; pti<1024; ++pti) {
			if(!(page_tables[pdi][pti] & PT_PRESENT))
				continue;

			pt[pti] = page_tables[pdi][pti];
		}
	}

	if(pages.size()){}

	for(auto const& x : pages) {
		uint32_t virt = x.s;

		uint16_t pdi = virt >> 22;
		uint16_t pti = (virt >> 12) & 0x3FF;

		uint32_t* pt = (uint32_t*)paging_allocPages(1);
		if(!(pd[pdi] & PD_PRESENT))
			pd[pdi] = (uint32_t)pt | PD_RW | PD_PRESENT;

		if(pt[pti] & PT_PRESENT) {
			printf(" {{ Page (%d:%d) already present. That is VERY wrong. Should move the kernel up. }} ", pdi, pti);
			while(true) {}
		}

		// SHOULD MARK AS USER.
		pt[pti] = x.f | PT_PRESENT | PT_RW;
	}

	return ret;
}
