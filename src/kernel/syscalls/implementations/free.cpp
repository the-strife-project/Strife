#include <kernel/syscalls/syscalls.hpp>

void syscall_free(Scheduler& sched, uint32_t virt, size_t n) {
	Task& task = tasks[sched.getCurrent()].task;

	while(n--) {
		if(task.heap.find(virt) == task.heap.end())
			sched.kill(Scheduler::KillReason::BAD_FREE);

		Paging::Page p = task.paging->get(virt);
		task.pages.erase(p);
		task.heap.erase(virt);
		p.free();
		kernelPaging.free((uint32_t)p.phys);
		virt += PAGE_SIZE;
	}

	sched.resume();
}
