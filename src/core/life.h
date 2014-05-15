#ifndef INCLUDED_SCHEDULE
#define INCLUDED_SCHEDULE

#include <x86/descriptors.h>
#include <core/universe.h>

#define MAX_THREADS 1024
#define STACK_PAGES 16
#define STACK_SIZE  (STACK_PAGES * PAGE_SIZE)
#define STACK_START(slot) (-((1+(slot))*STACK_SIZE))
#define STACK_PAGE(s,i) ((void*)(s-(i+1)*PAGE_SIZE))

typedef int Semaphore;
typedef struct {
  int seconds, millis;
} Time;
typedef union {
  Semaphore* sem;
  Time time;
} Trigger;
typedef struct Life {
  struct Life* next;
  struct Life* prev;
  TSS* tss;
  Universe* univ;
  Universe* from;
  int slot;
  Trigger trig;
} Life;

extern Feature _schedule_;
extern Life activeRoot,pendingRoot,waitingRoot;

extern Selector scheduleGate;

Life* getLife(Selector gate);

void scheduleNext(TSS* cur);
void releaseSem(Semaphore*,int);
void setTimerFreq(int hz);

#endif
