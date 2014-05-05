#ifndef INCLUDED_SCHEDULE
#define INCLUDED_SCHEDULE

#include <cpu/descriptors.h>
#include <core/universe.h>

#define MAX_THREADS 1024
#define STACK_PAGES 16
#define STACK_SIZE  (STACK_PAGES * PAGE_SIZE)
#define STACK_START(slot) (-((1+(slot))*STACK_SIZE))
#define STACK_PAGE(s,i) (s-(i+1)*PAGE_SIZE)

typedef int Semaphore;
typedef struct {
  int seconds, millis;
} Time;
typedef union {
  Semaphore* sem;
  Time time;
} Trigger;
typedef struct Task {
  struct Task* next;
  struct Task* prev;
  TSS* tss;
  Universe* univ;
  Universe* from;
  int slot;
  Trigger trig;
} Task;

extern Feature _schedule_;
extern Task activeRoot,pendingRoot,waitingRoot;

extern Selector scheduleGate;

Task* getTask(Selector gate);

void scheduleNext(TSS* cur);
void releaseSem(Semaphore*,int);
void setTimerFreq(int hz);

#endif
