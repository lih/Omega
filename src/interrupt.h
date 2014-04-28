#ifndef INCLUDED_INTERRUPT
#define INCLUDED_INTERRUPT

#include "memory.h"

typedef struct {
  dword gs, fs, es, ds;				/* pushed the segs last */
  dword edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
  dword int_no, err_code;			/* our 'push byte #' and ecodes do this */
  dword eip, cs, eflags, useresp, ss;		/* pushed by the processor automatically */ 
} PACKED IDTParams;
typedef void(*IDTHandler)(IDTParams*);
typedef void(*SyscallHandler)(struct TSS*);

extern Feature _interrupts_,_exceptions_,_irqs_,_syscalls_;

extern IDTHandler irqs[16];
extern SyscallHandler syscalls[16];

void enableInterrupts();
void disableInterrupts();

#endif
