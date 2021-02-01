#ifndef MPI_HPP
#define MPI_HPP

#include <kernel/tasks/scheduler/scheduler.hpp>

// stdlib
#include <MPI>

typedef size_t CID_t;

struct ClientServer {
	std::PID_t client;
	std::PID_t server;
	// When the two processes close(), it is removed from the channels list.
	bool closedOnce;

	ClientServer()
		: client(NULL_PID), server(NULL_PID), closedOnce(false)
	{}

	ClientServer(std::PID_t client, std::PID_t server)
		: client(client), server(server), closedOnce(false)
	{}
};

void publish(std::PID_t pid, string str);
std::PID_t discover(string str);
void listen(Scheduler& sched, std::PID_t source);
void connect(Scheduler& sched, std::PID_t target);
void transfer(Scheduler& sched, CID_t cid, std::PID_t target);
void listenTransfer(Scheduler& sched);
void close(Scheduler& sched, std::PID_t current, CID_t cid, bool isKernel=false);

void clearTransfers(std::PID_t pid);
ClientServer& getChannel(CID_t cid);
bool isCIDvalid(CID_t cid);

void send(Scheduler& sched, CID_t cid, size_t tag, size_t sz, void* data);
void recv(Scheduler& sched, CID_t cid, size_t sz, void* data);
void onClose(const ClientServer& cs);

#endif
