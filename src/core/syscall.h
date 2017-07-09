#ifndef INCLUDED_SYSCALL
#define INCLUDED_SYSCALL

#include <core/universe.h>
#include <core/life.h>

extern Feature _syscalls_;
typedef void (*BIOSFun)();

void sys_die(Life*);
void sys_spark(Life*);
void sys_warp(Life*);
void sys_alloc(Life*);
void sys_acquire(Life*);
void sys_release(Life*);
void sys_mapTo(Life*);
void sys_mapFrom(Life*);
void sys_spawn(Life*);
void sys_anihilate(Life*);
void sys_wait(Life*);
void sys_bios(Life*);

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
