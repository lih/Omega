#ifndef INCLUDED_SYSCALL
#define INCLUDED_SYSCALL

#include "schedule.h"

extern Feature _syscalls_;

void sys_die(Task*);
void sys_spawn(Task*);
void sys_warp(Task*);
void sys_alloc(Task*);

dword syscall_die();
dword syscall_spawn(dword eip);
dword syscall_warp(Universe* u,dword eip);

#endif
