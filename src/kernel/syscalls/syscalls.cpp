#include <kernel/syscalls/syscalls.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/MPI/MPI.hpp>
#include <kernel/klibc/stdio>
#include <kernel/tasks/scheduler/scheduler.hpp>
#include <kernel/TSS/TSS.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/asm.hpp>

inline void shouldntHaveReturned(const string& name) {
	printf(" {{ jlxip's fault: %S() returned?!?!?!?!?! }} ", name);
}

void syscall_handler(uint32_t esp) {
	PushadRegs* regs = ISR_get_PushadRegs(esp);
	iretValues* iret = ISR_get_iretValues(esp);
	tasks[schedulers[0].getCurrent()].task.update(regs, iret);

	uint32_t id = regs->eax;
	string str;
	Scheduler& thisSched = schedulers[0];
	std::PID_t current = thisSched.getCurrent();

	switch(id) {
		case _SYSCALL_SEND:
			send(schedulers[0], regs->edi, regs->esi, regs->edx, (void*)(regs->ebx));
			shouldntHaveReturned("send");
			while(true);
		case _SYSCALL_RECV:
			recv(schedulers[0], regs->edi, regs->esi, (void*)(regs->edx));
			printf("\tTODO\n");
			while(true);
		case _SYSCALL_FORK:
			schedulers[0].fork();
			shouldntHaveReturned("fork");
			while(true);
		case _SYSCALL_DIE:
			schedulers[0].die(regs->edi);	// edi contains exit code.
			shouldntHaveReturned("die");
			while(true);
		case _SYSCALL_EXEC:
			syscall_exec(sanitizeString(current, (const char*)regs->edi));
			shouldntHaveReturned("exec");
			while(true);
		case _SYSCALL_WAIT:
			thisSched.wait(regs->edi);
			shouldntHaveReturned("wait");
			while(true);
		case _SYSCALL_PUBLISH:
			publish(current, sanitizeString(current, (const char*)regs->edi));
			shouldntHaveReturned("publish");
			while(true);
		case _SYSCALL_DISCOVER:
			regs->eax = discover(sanitizeString(current, (const char*)regs->edi));
			return;	// Will allways return.
		case _SYSCALL_LISTEN:
			listen(schedulers[0], regs->edi);
			shouldntHaveReturned("listen");
			while(true);
		case _SYSCALL_CONNECT:
			connect(schedulers[0], regs->edi);
			shouldntHaveReturned("connect");
			while(true);
		case _SYSCALL_TRANSFER:
			transfer(schedulers[0], regs->edi, regs->esi);
			return;	// Can perfectly return. No registers modified.
		case _SYSCALL_LISTEN_TRANSFER:
			listenTransfer(schedulers[0]);
			shouldntHaveReturned("listenTransfer");
			while(true);
		case _SYSCALL_CHANNEL_CLOSE:
			close(schedulers[0], schedulers[0].getCurrent(), regs->edi);
			return;	// Can return.
		case _SYSCALL_ALLOC:
			syscall_alloc(schedulers[0], regs->edi);
			shouldntHaveReturned("alloc");
			while(true);
		case _SYSCALL_FREE:
			syscall_free(schedulers[0], regs->edi, regs->esi);
			shouldntHaveReturned("free");
			while(true);
		default:
			schedulers[0].kill(Scheduler::KillReason::BAD_SYSCALL);
			shouldntHaveReturned("kill");
			while(true);
	}
}

void syscalls_init() {
	IDT_setPL(SYSCALL_INTNO, 3);	// Enable this interrupt in usermode.
	ISR_delegate(SYSCALL_INTNO, syscall_handler);
}
