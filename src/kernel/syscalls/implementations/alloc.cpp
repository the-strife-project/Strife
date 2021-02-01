#include <kernel/syscalls/syscalls.hpp>

void syscall_alloc(Scheduler& sched, size_t n) {
	Task& task = tasks[sched.getCurrent()].task;

	uint32_t ret = task.paging->alloc(n);
	uint32_t current = ret;

	// TODO: This should have a limit. Which?

	while(n--) {
		Paging::Page p = task.paging->get(ret);
		p.phys = kernelPaging.calloc();
		p.user = true;
		p.ro = false;
		p.exe = false;
		p.flush();
		task.pages.insert(p);
		task.heap.insert(ret);
		current += PAGE_SIZE;
	}

	task.regs.eax = ret;
	sched.resume();
}
