#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <heap>
#include <kernel/tasks/task.hpp>
#include <kernel/drivers/clock/clock.hpp>
#include <syscalls>	// stdlib

#define SCHEDULER_MAX_PRIO 20
#define QUANTUM 25

#define SCHEDULER_WEIGHT_INC 10
#define SCHEDULER_WEIGHT_FACTOR 2

#define NULL_PID 0
#define INIT_PID 1

#define KILLED_RET_VALUE (~0)

// Each core has a Scheduler instance.
class Scheduler {
public:
	struct KillReason {
		enum {
			USER_REQUEST,
			SEGFAULT_SANITIZE,
			EXEC_NOTFOUND,
			LIB_NOTFOUND,
			FUNC_NOTFOUND,
			BAD_SYSCALL,
			WAIT_NOCHILD,
			PUBLIC_ALREADY,
			CONNECT_FAIL,
			INVALID_CID,
			TRANSFER_NOT_CHILD,
			CHANNEL_ITSELF,
			BAD_MSG,
			EMPTY_MSG,
			BAD_FREE
		};
	};

	enum class SchedulerState : uint8_t {
		READY,	// Running or runnable.
		ZOMBIE,	// Parent has not waited yet.
		DEAD
	};

	struct SchedulerTask {
		std::PID_t parent;
		set<std::PID_t> children;
		// Siblings?

		uint8_t prio;	// From 0 (not a priority) to SCHEDULER_MAX_PRIO (urgent!)
		size_t weight;	// Priority weight (factor to the QUANTUM)
		size_t vruntime;	// ms

		Task task;	// Low level information.
		SchedulerState state;

		// TODO: UID, GID, EUID, EGID.
		uint32_t retValue;
		bool orphan;

		std::PID_t waiting;	// PID of the process currently waiting for. Zero if none.

		set<std::PID_t> channelRequests;
		set<size_t> openChannels;

		// Methods.
		SchedulerTask()
			: parent(0), prio(0), weight(0), vruntime(0), state(SchedulerState::ZOMBIE),
			  retValue(~0), orphan(true), waiting(NULL_PID)
		{}

		inline bool operator<(const SchedulerTask& other) const {
			return vruntime < other.vruntime;
		}

		inline void updateWeight() {
			weight = ((SCHEDULER_MAX_PRIO - prio) + SCHEDULER_WEIGHT_INC) * SCHEDULER_WEIGHT_FACTOR;
		}
	};

private:
	typedef pair<size_t, std::PID_t> TaskForHeap;	// (vruntime, PID)

	std::PID_t current;
	Heap<TaskForHeap> ready;
	size_t preemptTimerID;
	uint64_t ms;	// Global milliseconds at start of current burst.

public:
	Scheduler()
		: current(NULL_PID), preemptTimerID(0)
	{}

	void init(const Task& task);
	void fork();
	void exec(const Task& task);
	void die(uint32_t exitcode);
	void kill(size_t reason, string extra="", std::PID_t pid=0);
	void onExit(std::PID_t pid);

	void schedule(bool reinsert);
	void preempt();
	void block();
	void unblock(std::PID_t pid);
	void wait(std::PID_t pid);
	void cycle(size_t id);
	void resume();
	void updatevruntime();

	inline std::PID_t getCurrent() const {
		return current;
	}
};

extern vector<Scheduler> schedulers;
extern vector<Scheduler::SchedulerTask> tasks;

void initializeSchedulers(uint8_t cores, const Task& task);

#endif
