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

extern IDTHandler irqs[16];
extern IDTHandler syscalls[207];

void initInterrupts();

void enableInterrupts();
void disableInterrupts();

#endif
