#include <kernel/tasks/task.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/klibc/stdio>

Task createTask(PID_t pid, uint32_t priority, PID_t parent, const list<PhysVirt>& pages) {
	Task ret;
	ret.PID = pid;
	ret.priority = priority;
	ret.parent = parent;

	// Prepare page table.
	uint32_t* pd = (uint32_t*)paging_allocPages(1);
	ret.pageDirectory = pd;

	// Copy kernel's page table.
	for(uint16_t pdi=0; pdi<1024; ++pdi) {
		uint32_t* pt = (uint32_t*)paging_allocPages(1);
		pd[pdi] = (uint32_t)pt | PD_RW | PD_PRESENT;

		for(uint16_t pti=0; pti<1024; ++pti) {
			if(!(page_tables[pdi][pti] & PT_PRESENT))
				continue;

			pt[pti] = page_tables[pdi][pti];
		}
	}

	for(auto const& x : pages) {
		uint16_t pdi = x.virt >> 22;
		uint16_t pti = (x.virt >> 12) & 0x3FF;

		if(!(pd[pdi] & PD_PRESENT))
			pd[pdi] = (uint32_t)paging_allocPages(1) | PD_RW | PD_PRESENT;
		pd[pdi] |= PD_ALL_PRIV;
		uint32_t* pt = (uint32_t*)(pd[pdi] & ~0xFFF);

		if(pt[pti]) {
			printf(" {{ jlxip's fault: awful range }}");
			while(true) {}
		}

		pt[pti] = x.phys | PT_PRESENT | PT_RW | PT_ALL_PRIV;
	}

	return ret;
}
