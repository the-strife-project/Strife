#include <kernel/tasks/scheduler/scheduler.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/usermode/usermode.hpp>
#include <kernel/MPI/MPI.hpp>

// TODO: MOVE THIS TO MSS!!!!
const char* killReasons[] = {
	[Scheduler::KillReason::USER_REQUEST] = "user request",
	[Scheduler::KillReason::SEGFAULT_SANITIZE] = "segmentation fault via syscall",
	[Scheduler::KillReason::EXEC_NOTFOUND] = "exec file not found",
	[Scheduler::KillReason::LIB_NOTFOUND] = "needs library \"%S\"",
	[Scheduler::KillReason::FUNC_NOTFOUND] = "needs function \"%S\" from some (newer?) library",
	[Scheduler::KillReason::BAD_SYSCALL] = "bad syscall",
	[Scheduler::KillReason::WAIT_NOCHILD] = "tried to wait a non direct child process",
	[Scheduler::KillReason::PUBLIC_ALREADY] = "public name \"%S\" already assigned",
	[Scheduler::KillReason::CONNECT_FAIL] = "connection to an invalid PID",
	[Scheduler::KillReason::INVALID_CID] = "invalid channel ID",
	[Scheduler::KillReason::TRANSFER_NOT_CHILD] = "transfer to a non-child process",
	[Scheduler::KillReason::CHANNEL_ITSELF] = "channel to itself",
	[Scheduler::KillReason::BAD_MSG] = "not expected message size",
	[Scheduler::KillReason::EMPTY_MSG] = "empty message",
	[Scheduler::KillReason::BAD_FREE] = "attempted to free a non-allocated page"
};

// TODO: CHECK ALL SYSCALLS AND MAKE THEM CALL updatevruntime() IF NECESSARY

// COMMON TO ALL CORES
vector<Scheduler> schedulers;
vector<Scheduler::SchedulerTask> tasks;	// Index is PID

void initializeSchedulers(uint8_t cores, const Task& task) {
	schedulers = vector<Scheduler>();
	for(uint32_t i=0; i<cores; ++i)
		schedulers.push_back(Scheduler());

	tasks = vector<Scheduler::SchedulerTask>();
	schedulers[0].init(task);
}

std::PID_t _genPID() {
	// TODO: Change when multiple cores. Mutex.
	// Maybe there's a better way to do it (reusing PIDs) that still is O(1)?
	std::PID_t ret = tasks.size();
	tasks.push_back(Scheduler::SchedulerTask());
	return ret;
}

// CORE SPECIFIC
void Scheduler::init(const Task& task) {
	// Null task (PID 0). TODO THIS SHOULD ONLY BE DONE IN CPU0.
	_genPID();

	// Init task (PID INIT_PID). TODO SAME AS ABOVE.
	_genPID();

	SchedulerTask st;
	st.parent = 0;
	st.prio = 0;
	st.updateWeight();
	st.vruntime = 0;
	st.task = task;
	st.state = SchedulerState::READY;
	st.retValue = ~0;
	tasks[INIT_PID] = st;

	ready.push(TaskForHeap(0, INIT_PID));
	preemptTimerID = clock_addTimer(QUANTUM, true);

	schedule(false);
}

void Scheduler::fork() {
	printf("Forking...\n");

	std::PID_t pid = _genPID();

	SchedulerTask st;
	st.parent = current;
	tasks[current].children.insert(pid);
	st.prio = tasks[current].prio;
	st.weight = tasks[current].weight;
	st.vruntime = 0;
	st.state = Scheduler::SchedulerState::READY;
	st.orphan = false;

	// Create a brand new task.
	Task& currentTask = tasks[current].task;

	list<PhysVirt> newpages;
	for(auto const& x : currentTask.pages) {
		// TODO: COW would go here.
		PhysVirt aux;
		aux.phys = kernelPaging.alloc();
		aux.virt = x.getVirt();
		memcpy((void*)aux.phys, (void*)x.phys, PAGE_SIZE);
		newpages.push_back(aux);
	}

	Task newtask(newpages, currentTask.iret.eip);
	newtask.regs = currentTask.regs;
	newtask.iret = currentTask.iret;

	// Copy stack.
	newtask.stackPages = currentTask.stackPages;
	newtask.stack = currentTask.stack;
	uint32_t stackVirt = 0x90000000;
	for(uint32_t i=0; i<newtask.stackPages; ++i) {
		uint32_t newStackPage = kernelPaging.alloc();
		memcpy((void*)newStackPage, (void*)currentTask.getPhys(stackVirt), PAGE_SIZE);
		stackVirt -= PAGE_SIZE;
	}

	// Change registers of each process.
	currentTask.regs.eax = (size_t)0;
	currentTask.regs.ebx = pid;
	newtask.regs.eax = (size_t)1;
	newtask.regs.ebx = current;

	updatevruntime();

	// Done.
	st.task = newtask;
	tasks[pid] = st;
	ready.push(TaskForHeap(0, pid));
	schedule(true);
	resume();
}

void Scheduler::exec(const Task& t) {
	tasks[current].vruntime = 0;
	tasks[current].task.free();
	tasks[current].task = t;

	schedule(true);
	resume();
}

void Scheduler::die(uint32_t exitcode) {
	tasks[current].retValue = exitcode;
	tasks[current].state = Scheduler::SchedulerState::ZOMBIE;
	printf("PID %x exited with code %d\n", current, exitcode);

	onExit(current);
	schedule(false);
	resume();
}

void Scheduler::kill(size_t reason, string extra, std::PID_t pid) {
	if(!pid)
		pid = current;

	tasks[pid].retValue = KILLED_RET_VALUE;
	tasks[pid].state = Scheduler::SchedulerState::ZOMBIE;

	// TODO: Should be in neat red.
	// TODO: Make printf receive a string (for fuck's sake).
	string format = "\nKilled process %d.\n";
	format += "Reason: ";
	format += killReasons[reason];
	format += ".\n";
	char* cstr = new char[format.length()+1];
	for(size_t i=0; i<format.length(); ++i)
		cstr[i] = format[i];
	cstr[format.length()] = 0;
	printf(cstr, pid, extra);

	onExit(pid);

	if(pid == current) {
		schedule(false);
		resume();
	}
}

void Scheduler::onExit(std::PID_t pid) {
	tasks[pid].task.free();

	for(auto const& x : tasks[pid].children) {
		// Make all children orphan.
		tasks[x].orphan = true;

		// Kill all zombies.
		if(tasks[x].state == SchedulerState::ZOMBIE)
			tasks[x].state = SchedulerState::DEAD;
	}

	// If the process is an orphan, kill it.
	if(tasks[pid].orphan) {
		tasks[pid].state = Scheduler::SchedulerState::DEAD;
	} else {
		// Is the parent waiting?
		SchedulerTask& parent = tasks[tasks[pid].parent];
		if(parent.waiting == pid) {
			parent.waiting = NULL_PID;
			parent.task.regs.eax = tasks[pid].retValue;
			unblock(tasks[pid].parent);
		}
	}

	// Close all open channels.
	for(auto x : tasks[pid].openChannels)
		close(*this, pid, x, true);

	clearTransfers(pid);
}

void Scheduler::schedule(bool reinsert) {
	if(current && reinsert)
		ready.push(TaskForHeap(tasks[current].vruntime, current));

	current = NULL_PID;
	while(!ready.empty()) {
		auto pid = ready.top().s;
		ready.pop();

		SchedulerState state = tasks[pid].state;

		// Dead or zombie? Take it off "ready".
		if(state != SchedulerState::DEAD && state != SchedulerState::ZOMBIE) {
			current = pid;
			break;
		}
	}
}

void Scheduler::preempt() {
	if(current) {
		updatevruntime();
		schedule(true);
		if(!current) {
			// If preempted, at least current process should be ready.
			printf(" {{ jlxip's fault: WTF?!?! }} ");
			while(true) {}
		}
	}
}

void Scheduler::block() {
	if(!current) {
		printf(" {{ jlxip's fault: scheduler exception }} ");	// TODO: Remove this once I check it doesn't happen.
		while(true) {}
	}

	SchedulerTask& curr = tasks[current];
	updatevruntime();
}

void Scheduler::unblock(std::PID_t pid) {
	SchedulerTask& st = tasks[pid];
	st.state = SchedulerState::READY;

	// TODO: Which scheduler so this go to? Assuming "this" right now.
	ready.push(TaskForHeap(st.vruntime, pid));

	// TODO RIGHT NOW: Increment time of current.
}

void Scheduler::wait(std::PID_t pid) {
	if(tasks[pid].parent != current)
		kill(KillReason::WAIT_NOCHILD);

	printf("PID %x va a esperar a %x\n", current, pid);

	SchedulerTask& child = tasks[pid];
	if(child.state == Scheduler::SchedulerState::ZOMBIE) {
		tasks[current].task.regs.eax = child.retValue;
		child.state = Scheduler::SchedulerState::DEAD;
	} else {
		tasks[current].waiting = pid;
		block();
		schedule(false);
	}
	resume();
}

void Scheduler::cycle(size_t id) {
	if(id == preemptTimerID) {
		preempt();
	} else {
		printf("What is this (%d)?\n", id);
		while(true) {}
	}
}

void Scheduler::resume() {
	if(current) {
		printf("R%d ", current);
		tasks[current].task.paging->use();
		// Substract one so that at least 1 ms passes when modifying vruntime.
		ms = clock_getMs() - 1;
		bochs_breakpoint();
		jump_usermode(tasks[current].task.regs, tasks[current].task.iret);
	} else {
		// TODO: cpu_relax();
		printf("Nada que hacer.\n");
		while(true);
	}
}

void Scheduler::updatevruntime() {
	if(!current) {
		printf(" {{ jlxip's fault: bad updatevruntime() }} ");
		while(true);
	}

	tasks[current].vruntime += (clock_getMs() - ms) * tasks[current].weight;
	ms = clock_getMs();	// Just in case... TODO REMOVE THIS
}
