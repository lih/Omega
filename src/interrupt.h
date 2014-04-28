#ifndef INCLUDED_INTERRUPT
#define INCLUDED_INTERRUPT

#include "memory.h"

typedef struct {
  byte _fourteen:5;
  byte dpl:2;
  byte present:1;
} __attribute__((__packed__)) IDTFlags;
typedef struct {
  word base_lo;
  word sel;
  byte _zero;
  IDTFlags flags;
  word base_hi;
} __attribute__((__packed__)) IDTEntry;
typedef struct {
  dword gs, fs, es, ds;				/* pushed the segs last */
  dword edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
  dword int_no, err_code;			/* our 'push byte #' and ecodes do this */
  dword eip, cs, eflags, useresp, ss;		/* pushed by the processor automatically */ 
} __attribute__((__packed__)) IDTParams;
typedef void(*IDTHandler)(IDTParams*);
typedef void(*SyscallHandler)(struct TSS*);

extern IDTEntry idts[256];
extern IDTHandler irqs[16];
extern SyscallHandler syscalls[16];

void initInterrupts();

void enableInterrupts();
void disableInterrupts();

IDTEntry idtEntry(dword base,word sel,byte dpl,byte present);
IDTEntry tssGate(word tssSel,byte dpl);

#endif
