#ifndef INCLUDED_PERVASIVES
#define INCLUDED_PERVASIVES

#include <util/memory.h>

void	shutdown();
void	nop();

void    loadInterrupts();

void	enablePaging();
void	disablePaging();
void    setPageDirectory(void* pd);

void    setLifeRegister(word reg);
word    getLifeRegister();

byte	inportb(word _port);
void	outportb(word _port,byte _msg);
word	inportw(word _port);
void	outportw(word _port,word _msg);

void    flushGDT();
dword   getPL();

dword compareAndSet(dword* addr,dword val);

void realMode(void (*callback)());
dword getIP();

#endif
