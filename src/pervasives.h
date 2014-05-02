#ifndef INCLUDED_PERVASIVES
#define INCLUDED_PERVASIVES

#include "memory.h"

void	shutdown();
void	nop();

void    loadInterrupts();

void	enablePaging();
void	disablePaging();
void    setPageDirectory(void* pd);

void    setTaskRegister(word reg);
word    getTaskRegister();

byte	inportb(word _port);
void	outportb(word _port,byte _msg);
word	inportw(word _port);
void	outportw(word _port,word _msg);

void    flushGDT();
void    getPL();

#endif
