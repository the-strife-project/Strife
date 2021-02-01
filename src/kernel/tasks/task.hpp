#ifndef TASK_HPP
#define TASK_HPP

#include <set>
#include <list>
#include <pair>
#include <kernel/loader/loader.hpp>	// For PhysVirt.
#include <queue>
#include <MPI>
#include <kernel/asm.hpp>
#include <kernel/paging/paging.hpp>

#define ALWAYS_ONE_FLAG_MASK 0x0002
#define INSTRUCTION_FLAG_MASK 0x0200

// Low level task.
struct Task {
	bool isFree;	// Whether this task has been freed (and, therefore, can be reused).
	Paging* paging;
	set<Paging::Page> pages;
	uint32_t stack;	// Virtual address of the stack pointer at beginning of execution.
	uint32_t stackPages;	// Number of pages of the stack.
	set<size_t> heap;	// Virtual addresses of allocated pages in the heap.

	// IO here

	// Manually pushed general registers on interrupt.
	struct Regs {
		size_t eax, ebx, ecx, edx;
		size_t esi, edi, ebp;
	} regs;

	iretValues iret;

	Task() {}
	Task(const list<PhysVirt>& _pages, size_t entryPoint);

	uint32_t getPhys(uint32_t virt) const;

	// On interrupt.
	void update(PushadRegs* pushadRegs, iretValues* iv);

	void free();
};

#endif
