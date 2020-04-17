#include <kernel/syscalls/syscalls.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/IDT/IDT.hpp>

void syscalls_init(void) {
	IDT_SET_ENT(IDT[SYSCALL_INT], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_syscall, 3);
}

extern "C" uint32_t syscall_handler(uint32_t id, uint32_t arg0, uint32_t arg1, uint32_t arg2/*, uint32_t arg3*/) {
	switch(id) {
		case _SYS_WRITE:
			return sys_write(arg0, (char*)arg1, arg2);
		case _SYS_TMP_TEST:
			sys_tmp_test();
	}

	return ~0;
}
