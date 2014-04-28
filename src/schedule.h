#ifndef INCLUDED_SCHEDULE
#define INCLUDED_SCHEDULE

#include "descriptors.h"
#include "universe.h"
#include "interrupt.h"

typedef struct Task {
  TSS tss;
  /* Additional fields needed to identify tasks */
  struct RR* rr;
} PACKED Task;
typedef struct RR {
  struct RR* next;
  struct RR* prev;
  Task* task;
  Universe* univ;
  int slot;
} RR;

extern Feature _schedule_;

extern int timerPhase;
extern int seconds,millis;
extern word rootGate;

Task* newTask();
Task* getTask();

void setTimerFreq(int hz);

dword syscall_die();
dword syscall_spawn(dword eip);
dword syscall_warp(Universe* u,dword eip);

#endif
