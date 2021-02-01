#ifndef SYSCALLS_H
#define SYSCALLS_H

#define SYSCALL_INTNO 0x69
#include <syscalls>
#include <string>
#include <kernel/tasks/scheduler/scheduler.hpp>
#include <MPI>

void syscalls_init();
string sanitizeString(std::PID_t pid, const char* arr);
void syscall_exec(const string& path);
void syscall_alloc(Scheduler& sched, size_t n);
void syscall_free(Scheduler& sched, uint32_t virt, size_t n);

#endif
