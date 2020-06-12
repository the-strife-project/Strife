#include <kernel/tasks/schedulers/FCFS/FCFS.hpp>

FCFS::FCFS() {
	Task nullTask;
	nullTask.PID = 0;
	tasks.push(nullTask);
}

void FCFS::fork() {
	// Update context.
	tasks.push(tasks.top());
	// Set the new context. SAME STACK AND EVERYTHING. Do I have to change anything at all?
	// Context switch would be here.
}

void FCFS::kill() {
	tasks.pop();
}
