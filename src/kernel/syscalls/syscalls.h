#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <common/types.h>

extern "C" void IDT_syscall(void);
void syscalls_init(void);
#define SYSCALL_INT 0x69

#define _SYS_READ 0

#define _SYS_WRITE 1
uint32_t sys_write(uint32_t fd, char* buf, uint32_t count);

// These are to be removed in the future.
#define _SYS_TMP_TEST 0xCACABACA
void sys_tmp_test();

#endif
