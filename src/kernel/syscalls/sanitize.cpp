#include <kernel/syscalls/syscalls.hpp>
#include <kernel/tasks/scheduler/scheduler.hpp>

string sanitizeString(std::PID_t pid, const char* arr) {
	string ret;
	size_t i = 0;

	Task& task = tasks[pid].task;
	bool checkedOnce = false;
	uint32_t lastPageChecked = 0;
	const char* realpath = (const char*)task.getPhys((uint32_t)arr);
	while(true) {
		uint32_t thisPage = (uint32_t)(arr+i) & ~0xFFF;
		if(!checkedOnce || lastPageChecked != thisPage) {
			uint32_t phys = task.getPhys(thisPage);
			if(!phys) {
				// Page is not on the page table, or its DPL is not 3.
				schedulers[0].kill(Scheduler::KillReason::SEGFAULT_SANITIZE);
			}
			lastPageChecked = thisPage;
			checkedOnce = true;
		}

		// It's safe now to dereference.
		char c = *(realpath+i);
		if(!c)
			break;

		ret += c;
		++i;
	}

	return ret;
}
