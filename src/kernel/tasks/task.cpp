#include <kernel/tasks/task.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/klibc/stdio>
#include <kernel/GDT/GDT.hpp>

uint32_t Task::getPhys(uint32_t virt) const {
	if(isFree) {
		printf(" {{ jlxip's fault: called getPhys() on freed task. }} ");
		while(true);
	}

	Paging::Page p = paging->get(virt);
	if(!p.present || !p.user)
		return 0;

	return p.phys + (virt & 0xFFF);
}

Task::Task(const list<PhysVirt>& _pages, size_t entryPoint) {
	// Prepare page table.
	uint32_t* pd = (uint32_t*)kernelPaging.calloc();
	paging = new Paging(pd);
	paging->copyFrom(kernelPaging);

	// Prepare stack.
	uint32_t st = kernelPaging.alloc();
	stack = 0x90000000;

	Paging::Page stack_p(paging, st, stack);
	stack_p.present = true;
	stack_p.ro = false;
	stack_p.user = true;
	stack_p.cow = false;
	stack_p.exe = false;
	stack_p.flush();

	pages.insert(stack_p);
	stack += PAGE_SIZE;
	stackPages = 1;
	// TODO: should push arguments here.
	// TODO: on page fault (not present, one page minus than "stack"), get more of it. Up to a limit, that is.

	for(auto const& x : _pages) {
		Paging::Page p(paging, x.phys, x.virt);
		p.present = true;
		p.user = true;
		p.ro = x.ro;
		p.cow = x.cow;
		p.exe = x.exe;
		p.flush();
		pages.insert(p);
	}

	regs.eax = regs.ebx = regs.ecx = regs.edx = 0;
	regs.esi = regs.edi = regs.ebp = 0;

	iret.eip = entryPoint;
	iret.cs = _USER_CODESEGMENT | 0b11;
	iret.flags = ALWAYS_ONE_FLAG_MASK | INSTRUCTION_FLAG_MASK;
	iret.esp = stack;
	iret.ss = _USER_DATASEGMENT | 0b11;

	isFree = false;
}

void Task::update(PushadRegs* pushadRegs, iretValues* iv) {
	regs.eax = pushadRegs->eax;
	regs.ebx = pushadRegs->ebx;
	regs.ecx = pushadRegs->ecx;
	regs.edx = pushadRegs->edx;
	regs.esi = pushadRegs->esi;
	regs.edi = pushadRegs->edi;
	regs.ebp = pushadRegs->ebp;
	iret = *iv;
}

void Task::free() {
	if(!isFree) {
		for(auto& x : pages)
			x.free();

		paging->destroyAfterFree();
		isFree = true;
	} else {
		printf(" {{ jlxip's fault: FREED TWICE!!! }} ");
		while(true);
	}
}
