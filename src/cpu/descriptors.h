#ifndef INCLUDED_DESCRIPTORS
#define INCLUDED_DESCRIPTORS

#include <cpu/memory.h>

typedef word Selector;
typedef dword DirEntry;

typedef struct {
  dword cf:1;
  dword _one:1;
  dword pf:1;
  dword _zero:1;
  dword af:1;
  dword _zero2:1;
  dword zf:1;
  dword sf:1;
  dword tf:1;
  dword _if:1;
  dword df:1;
  dword of:1;
  dword iopl:2;
  dword nt:1;
  dword _zero3:1;
  dword rf:1;
  dword vm:1;
  dword ac:1;
  dword vif:1;
  dword vip:1;
  dword id:1;
  dword _zero4:10;
} PACKED EFLAGS;
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
  dword cr3, eip;
  EFLAGS eflags;
  dword eax,ecx,edx,ebx,esp,ebp,esi,edi;
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
#define TSS_AT(sel) (descBase(DESCRIPTOR_AT(gdt,sel)))
Selector addDesc(TablePtr* tbl,Descriptor desc);
void*    descBase(Descriptor desc);

Descriptor tssDesc(TSS* tss,byte busy);
Descriptor taskGate(Selector tssSel,byte dpl);
Descriptor interruptGate(Selector sel,void* offset,byte dpl);

TSS tss(Dir* pageDir,dword eip,dword esp);

#endif
