#ifndef SCHEDULERS_COMMON
#define SCHEDULERS_COMMON

#include <kernel/tasks/task.hpp>

void contextSwitch(const Task& task);

#endif
