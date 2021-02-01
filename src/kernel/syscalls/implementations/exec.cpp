#include <kernel/syscalls/syscalls.hpp>
#include <kernel/tasks/scheduler/scheduler.hpp>
#include <kernel/mounts/mounts.hpp>
#include <kernel/paging/paging.hpp>

void syscall_exec(const string& path) {
	// TODO: Use a scheduler passed by parameter
	FSRawChunk chunk = readFile(path);
	if(!chunk.good())
		schedulers[0].kill(Scheduler::KillReason::EXEC_NOTFOUND);

	Program p;
	p.parse(chunk.get());
	if(!p.loadDynamicLibraries())
		schedulers[0].kill(Scheduler::KillReason::LIB_NOTFOUND, p.getFailedDynamicLibrary());
	p.load();
	p.relocate();

	Task t(p.getPages(), p.getEntryPoint());
	t.paging->use();
	if(!p.relocate2())
		schedulers[0].kill(Scheduler::KillReason::FUNC_NOTFOUND, p.getFailedRelocation());
	kernelPaging.use();

	chunk.destroy();
	schedulers[0].exec(t);
}
