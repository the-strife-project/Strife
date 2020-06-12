#ifndef FCFS_HPP
#define FCFS_HPP

#include <kernel/tasks/task.hpp>
#include <kernel/klibc/STL/stack>

class FCFS {
private:
	stack<Task> tasks;

public:
	FCFS();

	void fork();
	void exec(/* Some form of executable class or struct or something.*/);
	void kill();
};

#endif
