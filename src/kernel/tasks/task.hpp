#ifndef TASK_HPP
#define TASK_HPP

#include <kernel/klibc/STL/set>
#include <kernel/klibc/STL/list>
#include <kernel/klibc/STL/pair>

typedef uint32_t PID_t;

struct Task {
	PID_t PID;
	uint32_t priority;

	PID_t parent;
	set<PID_t> children;
	// Siblings?

	// File descriptors would be here.
	// UID, GID, EUID, EGID.

	// TSS here.
	uint32_t* pageDirectory;	// Delet this.

	Task()
		: PID(0), priority(0), parent(0), pageDirectory(nullptr)
	{}
};

Task createTask(PID_t pid, uint32_t priority, PID_t parent, const list<pair<uint32_t, uint32_t>>& pages);

#endif
