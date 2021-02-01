#include <kernel/MPI/MPI.hpp>
#include <syscalls>
#include <queue>

queue<CID_t> freedCIDs = queue<CID_t>();
vector<ClientServer> channels = vector<ClientServer>();

/*
	Processes that are blocked because they're listening.
	Key is PID of the process, value is:
		- NULL_PID if listening for any.
		- Some other value if listening for that specific PID.
*/
map<std::PID_t, std::PID_t> blockedListening = map<std::PID_t, std::PID_t>();
set<std::PID_t> blockedConnecting = set<std::PID_t>();

CID_t getCID() {
	// Insert null CID
	if(channels.size() == 0)
		channels.push_back(ClientServer());

	// Mutex.
	if(freedCIDs.empty()) {
		channels.push_back(ClientServer());
		return channels.size()-1;
	} else {
		CID_t ret = freedCIDs.front();
		freedCIDs.pop();
		return ret;
	}
}

// Match: client is connecting, server is listening.
void match(std::PID_t client, std::PID_t server) {
	CID_t cid = getCID();
	channels[cid] = ClientServer(client, server);
	tasks[client].task.regs.eax = cid;
	tasks[server].task.regs.eax = cid;
	tasks[client].openChannels.insert(cid);
	tasks[server].openChannels.insert(cid);
	return;
}

void listen(Scheduler& sched, std::PID_t source) {
	auto& cr = tasks[sched.getCurrent()].channelRequests;
	if(!cr.empty()) {
		std::PID_t client = NULL_PID;

		if(source == NULL_PID) {
			// Any source is fine.
			client = *(cr.begin());
			cr.erase(client);
		} else {
			// Must check source.
			auto it = cr.find(source);
			if(it != cr.end()) {
				client = *it;
				cr.erase(it);
			}
		}

		if(client) {
			match(client, sched.getCurrent());

			// Was it waiting?
			auto it = blockedConnecting.find(client);
			if(it != blockedConnecting.end()) {
				blockedConnecting.erase(it);
				sched.unblock(client);
			}

			sched.resume();
		}
	}

	// No valid message :(
	blockedListening[sched.getCurrent()] = source;
	sched.block();
	sched.schedule(false);
	sched.resume();
}

void connect(Scheduler& sched, std::PID_t target) {
	// Make all this mutex!

	if(target == sched.getCurrent())
		sched.kill(Scheduler::KillReason::CHANNEL_ITSELF);

	if(target >= tasks.size() ||
	   tasks[target].state == Scheduler::SchedulerState::ZOMBIE ||
	   tasks[target].state == Scheduler::SchedulerState::DEAD)
	{
		sched.kill(Scheduler::KillReason::CONNECT_FAIL);
	}

	tasks[target].channelRequests.insert(sched.getCurrent());

	// If it's blockedListening because of this, unblock it.
	auto it = blockedListening.find(pair<std::PID_t, std::PID_t>(target, NULL_PID));
	if(it != blockedListening.end()) {
		if((*it).s == NULL_PID || (*it).s == sched.getCurrent()) {
			blockedListening.erase(it);
			match(sched.getCurrent(), target);
			sched.unblock(target);
			sched.schedule(true);
			sched.resume();
		}
	}

	// No one listening. Block.
	blockedConnecting.insert(sched.getCurrent());
	sched.block();
	sched.schedule(false);
	sched.resume();
}

map<std::PID_t, queue<CID_t>> transfers = map<std::PID_t, queue<CID_t>>();
set<std::PID_t> blockedWaitTransfer = set<std::PID_t>();

void transfer(Scheduler& sched, CID_t cid, std::PID_t target) {
	if(target >= tasks.size() ||
	   tasks[target].state == Scheduler::SchedulerState::ZOMBIE ||
	   tasks[target].state == Scheduler::SchedulerState::DEAD)
	{
		sched.kill(Scheduler::KillReason::INVALID_CID);
	}

	if(tasks[target].parent != sched.getCurrent())
		sched.kill(Scheduler::KillReason::TRANSFER_NOT_CHILD);

	if(cid >= channels.size())
		sched.kill(Scheduler::KillReason::INVALID_CID);

	if(channels[cid].client == sched.getCurrent()) {
		channels[cid].client = target;
	} else if(channels[cid].server == sched.getCurrent()) {
		channels[cid].server = target;
	} else if(channels[cid].closedOnce) {
		tasks[sched.getCurrent()].task.regs.eax = (size_t)false;
		tasks[sched.getCurrent()].openChannels.erase(cid);
		sched.resume();
	} else {
		sched.kill(Scheduler::KillReason::INVALID_CID);
	}

	tasks[sched.getCurrent()].task.regs.eax = (size_t)true;
	tasks[sched.getCurrent()].openChannels.erase(cid);
	tasks[target].openChannels.insert(cid);

	// Was it waiting?
	auto it = blockedWaitTransfer.find(target);
	if(it != blockedWaitTransfer.end()) {
		blockedWaitTransfer.erase(it);
		tasks[target].task.regs.eax = cid;
		sched.unblock(target);
		sched.schedule(true);
	} else {
		transfers[target].push(cid);
	}

	sched.resume();
}

void listenTransfer(Scheduler& sched) {
	// Pending transfer?
	queue<CID_t>& thisTransfers = transfers[sched.getCurrent()];
	if(!thisTransfers.empty()) {
		tasks[sched.getCurrent()].task.regs.eax = thisTransfers.front();
		thisTransfers.pop();
		sched.resume();
	}

	blockedWaitTransfer.insert(sched.getCurrent());
	sched.block();
	sched.schedule(false);
	sched.resume();
}

void close(Scheduler& sched, std::PID_t current, CID_t cid, bool isKernel) {
	if(cid >= channels.size())
		sched.kill(Scheduler::KillReason::INVALID_CID);

	if(channels[cid].client == current) {
		if(!isKernel)
			tasks[current].openChannels.erase(cid);
		channels[cid].client = NULL_PID;
	} else if(channels[cid].server == current) {
		if(!isKernel)
			tasks[current].openChannels.erase(cid);
		channels[cid].server = NULL_PID;
	} else {
		sched.kill(Scheduler::KillReason::INVALID_CID);
	}

	if(!channels[cid].closedOnce)
		channels[cid].closedOnce = true;
	else
		freedCIDs.push(cid);

	onClose(channels[cid]);
}

ClientServer& getChannel(CID_t cid) {
	return channels[cid];
}

void clearTransfers(std::PID_t pid) {
	transfers[pid] = queue<CID_t>();
}

bool isCIDvalid(CID_t cid) {
	return cid < channels.size();
}
