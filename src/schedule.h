#ifndef INCLUDED_SCHEDULE
#define INCLUDED_SCHEDULE

#include "descriptors.h"
#include "universe.h"
#include "interrupt.h"

typedef struct TSS {
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
  /* Additional fields needed to identify tasks */
  struct RR* rr;
} __attribute__((__packed__)) TSS;
typedef struct RR {
  struct RR* next;
  struct RR* prev;
  TSS* tss;
  Universe* univ;
  int slot;
} RR;

extern int timerPhase;
extern int seconds,millis;
extern word rootGate;

Descriptor tssDesc(TSS* tss,byte dpl);
TSS        tss(Dir*,word,dword,word,dword,word,dword);
TSS*       newTSS();

TSS*   getTSS();

void initSchedule();

void setTimerFreq(int hz);

dword syscall_die();
dword syscall_spawn(dword eip);
dword syscall_warp(Universe* u,dword eip);

#endif
