#ifndef USERMODE_H
#define USERMODE_H

#include <kernel/tasks/task.hpp>

extern "C" void jump_usermode(Task::Regs, iretValues);

#endif
