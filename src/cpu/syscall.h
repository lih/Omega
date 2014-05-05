#ifndef INCLUDED_SYSCALL
#define INCLUDED_SYSCALL

#include <core/schedule.h>

extern Feature _syscalls_;

void sys_die(Task*);
void sys_spark(Task*);
void sys_warp(Task*);
void sys_alloc(Task*);
void sys_acquire(Task*);
void sys_release(Task*);
void sys_mapTo(Task*);
void sys_mapFrom(Task*);
void sys_spawn(Task*);
void sys_anihilate(Task*);
void sys_wait(Task*);

dword syscall_die();
dword syscall_spark(int u,dword eip);
dword syscall_warp(int u,dword eip);
dword syscall_alloc(void* page,int n);
dword syscall_acquire(void* addr);
dword syscall_release(void* addr,int n);
dword syscall_mapTo(int uindex,void* src,void* dst);
dword syscall_mapFrom(int uindex,void* src,void* dst);
dword syscall_spawn();
dword syscall_anihilate(int uindex);
dword syscall_wait(int seconds,int millis);

#endif
