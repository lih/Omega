#ifndef INCLUDED_SYSCALL
#define INCLUDED_SYSCALL

#include <core/universe.h>
#include <core/schedule.h>

extern Feature _syscalls_;
typedef void (*BIOSFun)();

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
void sys_bios(Task*);

void bios_mode13h();
void bios_mode03h();

void	  syscall_die		();
void	  syscall_spark		(UIndex u,void* eip);
void	  syscall_warp		(UIndex u,void* eip);
void	  syscall_alloc		(void* page,int n);
Semaphore syscall_acquire	(Semaphore* addr);
void	  syscall_release	(Semaphore* addr,int n);
void	  syscall_mapTo		(UIndex uindex,void* src,void* dst);
void	  syscall_mapFrom	(UIndex uindex,void* src,void* dst);
UIndex	  syscall_spawn		();
void	  syscall_anihilate	(UIndex uindex);
void	  syscall_wait		(int seconds,int millis);
void      syscall_bios          (int funno);

#endif
