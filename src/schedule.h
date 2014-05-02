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
  struct Task* next;
  struct Task* prev;
  TSS* tss;
  Universe* univ;
  Universe* from;
  int slot;
  byte* sem;
  int timerSecs, timerMillis;
} Task;

extern Feature _schedule_;
extern Task activeRoot,pendingRoot,waitingRoot;

extern Selector scheduleGate;

Task* getTask(Selector gate);

void setTimerFreq(int hz);

#endif
