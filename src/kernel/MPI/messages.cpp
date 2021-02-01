#include <kernel/MPI/MPI.hpp>
#include <kernel/klibc/stdio>
#include <kernel/paging/paging.hpp>

struct Message {
	Scheduler* sched;
	std::PID_t pid;	// Maybe I don't need this field? (TODO)
	CID_t cid;
	size_t sz;
	void* ptr;
	size_t tag;
	list<size_t> physPages;

	Message()
		: sched(nullptr), pid(NULL_PID), cid(0), sz(0), ptr(nullptr), tag(0)
	{}

	Message(Scheduler* sched, std::PID_t pid, CID_t cid, size_t sz, void* ptr, size_t tag)
		: sched(sched), pid(pid), cid(cid), sz(sz), ptr(ptr), tag(tag)
	{}
};

// Blocked receivers and senders.
map<std::PID_t, Message> pendingReceive = map<std::PID_t, Message>();
map<std::PID_t, Message> pendingSend = map<std::PID_t, Message>();

bool sanePages(const Message& msg, list<size_t>& physPages) {
	size_t npages = msg.sz / PAGE_SIZE;
	if(msg.sz % PAGE_SIZE)
		++npages;

	size_t data = (size_t)(msg.ptr) & ~0xFFF;

	for(size_t i=0; i<npages; ++i) {
		size_t phys = tasks[msg.pid].task.getPhys(data + i*PAGE_SIZE);
		if(!phys)
			return false;
		else
			physPages.push_back(phys);
	}

	return true;
}

// Also fills msg.physPages
void checkSaneMessageEnvironment(Message& msg, size_t retOnFailure) {
	if(!isCIDvalid(msg.cid))
		msg.sched->kill(Scheduler::KillReason::INVALID_CID, "", msg.pid);

	const ClientServer& cs = getChannel(msg.cid);

	if(!msg.sz)
		msg.sched->kill(Scheduler::KillReason::EMPTY_MSG, "", msg.pid);

	if(cs.client != msg.pid && cs.server != msg.pid)
		msg.sched->kill(Scheduler::KillReason::INVALID_CID, "", msg.pid);

	if(cs.closedOnce) {
		tasks[msg.pid].task.regs.eax = retOnFailure;
		msg.sched->updatevruntime();
		msg.sched->resume();
	}

	if(!sanePages(msg, msg.physPages))
		msg.sched->kill(Scheduler::KillReason::SEGFAULT_SANITIZE, "", msg.pid);
}

void copyMessage(const Message& msgRecv, const Message& msgSend) {
	auto itRecv = msgRecv.physPages.begin();
	auto itSend = msgSend.physPages.begin();
	size_t ptrRecv = (size_t)msgRecv.ptr;
	size_t ptrSend = (size_t)msgSend.ptr;

	size_t sz = msgRecv.sz;
	while(sz) {
		// Displacement in page.
		size_t offRecv = ptrRecv & 0xFFF;
		size_t offSend = ptrSend & 0xFFF;
		// Free bytes in page.
		size_t freeRecv = PAGE_SIZE - offRecv;
		size_t freeSend = PAGE_SIZE - offSend;

		// How many bytes to copy.
		size_t bytes = sz;
		if(freeRecv < bytes)
			bytes = freeRecv;
		if(freeSend < bytes)
			bytes = freeSend;

		// memcpy cannot be used because there's no guarantee that the address is aligned.
		uint8_t* dst = (uint8_t*)(*itRecv + offRecv);
		uint8_t* src = (uint8_t*)(*itSend + offSend);
		for(size_t i=0; i<bytes; ++i)
			dst[i] = src[i];

		sz -= bytes;
		ptrRecv += bytes;
		ptrSend += bytes;

		if((ptrRecv & ~0xFFF) != ((ptrRecv + bytes) & ~0xFFF))
			++itRecv;
		if((ptrSend & ~0xFFF) != ((ptrSend + bytes) & ~0xFFF))
			++itSend;
	}
}

void send(Scheduler& sched, CID_t cid, size_t tag, size_t sz, void* data) {
	Message msg(&sched, sched.getCurrent(), cid, sz, data, tag);
	checkSaneMessageEnvironment(msg, (size_t)false);

	const ClientServer& cs = getChannel(cid);
	std::PID_t other = (msg.pid == cs.client) ? cs.server : cs.client;
	if(!pendingReceive[other].pid) {
		// No one listening.
		pendingSend[msg.pid] = msg;
		sched.block();
		sched.schedule(false);
		sched.resume();
	}

	if(pendingReceive[other].sz != sz)
		sched.kill(Scheduler::KillReason::BAD_MSG, "", cs.client);	// ALWAYS kill the client.

	// First page.
	copyMessage(pendingReceive[other], msg);

	tasks[sched.getCurrent()].task.regs.eax = (size_t)true;
	tasks[other].task.regs.eax = tag;
	pendingReceive[other] = Message();
	sched.updatevruntime();
	sched.unblock(other);
	// TODO: Direct process switch.
	sched.resume();
}

void recv(Scheduler& sched, CID_t cid, size_t sz, void* data) {
	Message msg(&sched, sched.getCurrent(), cid, sz, data, 0);
	checkSaneMessageEnvironment(msg, MPI_TAG_CLOSED);

	const ClientServer& cs = getChannel(cid);
	std::PID_t other = (msg.pid == cs.client) ? cs.server : cs.client;
	if(!pendingSend[other].pid) {
		pendingReceive[msg.pid] = msg;
		sched.block();
		sched.schedule(false);
		sched.resume();
	}

	if(pendingSend[other].sz != sz)
		sched.kill(Scheduler::KillReason::BAD_MSG, "", cs.client);	// ALWAYS kill the client.

	copyMessage(msg, pendingSend[other]);
	tasks[other].task.regs.eax = (size_t)true;
	tasks[sched.getCurrent()].task.regs.eax = pendingSend[other].tag;
	pendingSend[other] = Message();
	sched.updatevruntime();
	sched.unblock(other);
	sched.resume();
}

void onClose(const ClientServer& cs) {
	// Is the other party sending or receiving?
	std::PID_t other;
	if(cs.client)
		other = cs.client;
	else if(cs.server)
		other = cs.server;
	else
		return;

	if(pendingReceive[other].pid) {
		pendingReceive[other].sched->unblock(other);
		pendingReceive[other] = Message();
		tasks[other].task.regs.eax = MPI_TAG_CLOSED;
	} else if(pendingSend[other].pid) {
		pendingSend[other].sched->unblock(other);
		pendingSend[other] = Message();
		tasks[other].task.regs.eax = (size_t)false;
	}
}
