#ifndef INCLUDED_DESCRIPTORS
#define INCLUDED_DESCRIPTORS

#include "memory.h"

typedef word Selector;
typedef dword DirEntry;

typedef struct {
  word limit_lo;
  word base_lo;
  byte base_mi;
  byte type:4;
  byte codeOrData:1;
  byte dpl:2;
  byte present:1;
  byte limit_hi:4;
  byte free:1;
  byte code64:1;		/* set for 64-bit code segment */
  byte opsize:1; 		/* set for 32-bit mode */
  byte granularity:1; 		/* set for big pages */
  byte base_hi;
} PACKED Descriptor;
typedef struct {
  word limit;
  Descriptor* base;
} PACKED TablePtr;
typedef struct {
  word previousTask, _1;
  dword esp0; word ss0,_2;
  dword esp1; word ss1,_3;
  dword esp2; word ss2,_4;
  dword cr3, eip, eflags,
    eax,ecx,edx,ebx,esp,ebp,esi,edi;
  word es,_5,cs,_6,ss,_7,
    ds,_8,fs,_9,gs,_10,
    ldt, _11;
  word trap:1;
  word _12:15;
  word iomap;
} PACKED TSS;

#define DIR_SIZE (PAGE_SIZE/sizeof(DirEntry))
typedef dword Dir[DIR_SIZE];

extern TablePtr gdt;
extern TablePtr idt;

extern Descriptor idts[256];

#define DESCRIPTOR_AT(t,sel) ((t).base[(sel)/sizeof(Descriptor)]) 
Selector addDesc(TablePtr* tbl,Descriptor desc);
void*    descBase(Descriptor desc);

Descriptor tssDesc(TSS* tss,byte dpl,byte busy);
Descriptor taskGate(Selector tssSel,byte dpl);
Descriptor interruptGate(Selector sel,void* offset,byte dpl);

TSS tss(Dir* pageDir,word ss0,dword esp0,
	word ss1,dword esp1,word ss2,dword esp2);

#endif
