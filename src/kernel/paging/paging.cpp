#include <kernel/paging/paging.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/boot.hpp>
#include <kernel/asm.hpp>
#include <kernel/klibc/stdlib.hpp>

uint32_t kernelPageDirectory[1024] __attribute__((aligned(4096)));
uint32_t kernelPageTables[1024][1024] __attribute__((aligned(4096)));


Paging::Page::Page()
	: virt(0), pdi(0), pti(0), pt(nullptr),
	  parent(nullptr), phys(0),
	  present(false), user(false),
	  ro(false), cow(false), exe(false),
	  dnf(false)
{}

Paging::Page::Page(Paging* parent, size_t phys, size_t virt)
	: virt(virt), parent(parent), phys(phys),
	  present(false), user(false),
	  ro(false), cow(false), exe(false),
	  dnf(false)
{
	pdi = virt >> 22;
	pti = (virt >> 12) & 0x3FF;

	// Is pt present? Not a huge deal if this reserves a few more pages than necessary. Should be avoided tho.
	if(!(parent->pageDirectory[pdi] & PD_PRESENT)) {
		if(parent->pageDirectory != kernelPageDirectory) {
			size_t newPageTable = kernelPaging.calloc();
			parent->pageDirectory[pdi] = newPageTable;
		}

		parent->pageDirectory[pdi] |= PD_PRESENT;
		// Always all privs to the directory.
		parent->pageDirectory[pdi] |= PD_RW;
		parent->pageDirectory[pdi] |= PD_ALL_PRIV;
	}

	pt = (uint32_t*)(parent->pageDirectory[pdi] & ~0xFFF);
	present = pt[pti] & PT_PRESENT;
	user = pt[pti] & PT_ALL_PRIV;
	ro = !(pt[pti] & PT_RW);
}

void Paging::Page::flush() {
	pt[pti] = phys;

	if(present)
		pt[pti] |= PT_PRESENT;
	if(user)
		pt[pti] |= PT_ALL_PRIV;
	if(!ro)
		pt[pti] |= PT_RW;

	invlpg(virt);
}

void Paging::Page::free() {
	present = false;
	pt[pti] &= ~PT_PRESENT;

	if(!(ro && cow) && !dnf)
		parent->free(phys, 1);
}



void Paging::copyFrom(Paging other) {
	for(uint16_t pdi=0; pdi<1024; ++pdi) {
		if(!(other.pageDirectory[pdi] & PD_PRESENT))
			continue;

		uint32_t* pt = (uint32_t*)(other.pageDirectory[pdi] & ~0xFFF);
		for(uint16_t pti=0; pti<1024; ++pti) {
			if(!(pt[pti] & PT_PRESENT))
				continue;

			size_t virt = (size_t)(pdi)*0x400000 + (size_t)(pti)*0x1000;
			Paging::Page p(this, pt[pti] & ~0xFFF, virt);
			p.getRaw() = other.get(virt).getRaw();
			// No need to flush.
		}
	}
}

uint32_t Paging::getPhys(uint32_t virt) {
	uint32_t pdi = virt >> 22;
	uint32_t pti = (virt >> 12) & 0x3FF;
	uint32_t* pt = (uint32_t*)(pageDirectory[pdi] & ~0xFFF);
	return pt[pti] & ~0xFFF;
}

uint32_t Paging::findConsecutive(size_t count) {
	// TODO: Optimize this bruteforce approach.
	uint32_t continuous = 0;
	uint32_t sd = 0;
	uint32_t sp = 0;

	// TODO: this should be num_pd and num_pt instead of 1024. This will eventually break.
	for(uint16_t i=0; i<1024; ++i) {
		for(uint16_t j=0; j<1024; ++j) {
			if(get((size_t)(i)*0x400000 + (size_t)(j)*0x1000).present) {
				// Not found yet.
				continuous = 0;
				sd = i;
				sp = j + 1;

				if(sp == 1024) {
					++sd;
					sp = 0;
				}
			} else {
				if(++continuous == count)
					return (sd * 0x400000) + (sp * 0x1000);
			}
		}
	}

	// ↓ That's a bad fucking idea
	panic(KP_OUTOFMEM);
	return 0;
}

uint32_t Paging::alloc(size_t count) {
	uint32_t ret = findConsecutive(count);
	uint32_t ptr = ret;

	while(count--) {
		Page p(this, ptr, ptr);
		p.present = true;
		p.ro = false;
		p.flush();
		ptr += PAGE_SIZE;
	}

	return ret;
}

void Paging::free(uint32_t virt, size_t count) {
	while(count--) {
		Page p(this, getPhys(virt), virt);
		p.present = false;
		p.flush();
		virt += PAGE_SIZE;
	}
}

void Paging::use() {
	asm volatile (
		"mov %%eax, %%cr3"
		:: "a" (pageDirectory)
		: "memory", "cc");
}


// Kernel specific.
Paging kernelPaging;

void paging_init() {
	kernelPaging = Paging(kernelPageDirectory);

	// Fill page directory with page tables.
	for(uint32_t i=0; i<1024; ++i) {
		kernelPageDirectory[i] = ((uint32_t)kernelPageTables[i]);

		// Initialize all pages to its physical address. NOT PRESENT.
		for(uint32_t j=0; j<1024; ++j)
			kernelPageTables[i][j] = i*0x400000 + j*0x1000;
	}

	// Kernel pages and everything below.
	uint32_t maxKernel = PAGE_SIZE + (uint32_t)ASM_KERNEL_END;
	for(uint32_t i=0; i<maxKernel+PAGE_SIZE-1; i+=PAGE_SIZE) {
		Paging::Page p(&kernelPaging, i, i);
		p.present = true;
		p.flush();
	}

	// Possible memory mapped hardware.
	for(uint32_t i=0x00F00000; i<=0x00FFFFFF; i+=PAGE_SIZE) {
		Paging::Page p(&kernelPaging, i, i);
		p.present = true;
		p.flush();
	}
}
