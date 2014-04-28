#ifndef INCLUDED_SCHEDULE
#define INCLUDED_SCHEDULE

#include "descriptors.h"
#include "universe.h"

#define MAX_THREADS 1024
#define STACK_PAGES 16
#define STACK_SIZE  (STACK_PAGES * PAGE_SIZE)
#define STACK_START(slot) (-((1+(slot))*STACK_SIZE))
#define STACK_PAGE(s,i) (s-(i+1)*PAGE_SIZE)


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

extern Selector scheduleGate;

Task* newTask();
Task* getTask();

void setTimerFreq(int hz);

#endif
