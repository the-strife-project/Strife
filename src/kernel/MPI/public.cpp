#include <kernel/MPI/MPI.hpp>
#include <map>
#include <kernel/tasks/scheduler/scheduler.hpp>

map<string, std::PID_t> publicNames = map<string, std::PID_t>();

#include <kernel/klibc/stdio>
void publish(std::PID_t pid, string str) {
	// Mutex.

	if(publicNames.find(pair<string, std::PID_t>(str, NULL_PID)) != publicNames.end())
		schedulers[0].kill(Scheduler::KillReason::PUBLIC_ALREADY, str);

	printf("%d se ha publicado como %S\n", pid, str);
	publicNames[str] = pid;

	schedulers[0].resume();
}

std::PID_t discover(string str) {
	// Same lock as above.

	if(publicNames.find(pair<string, std::PID_t>(str, NULL_PID)) == publicNames.end())
		return NULL_PID;

	return publicNames[str];
}
