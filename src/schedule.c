#include "constants.h"
#include "memory.h"
#include "descriptors.h"
#include "schedule.h"
#include "universe.h"
#include "framebuffer.h"
#include "feature.h"
#include "timer.h"

Pool taskPool = { NULL, sizeof(Task) };
Pool rrPool = { NULL, sizeof(RR) };

RR rr_root;
byte stackSlots[MAX_THREADS] = { 0 };
int  curSlot = 0;
Selector scheduleGate,slaveGate;

Universe rootSpace;

void schedule();
void yield();

static void initSchedule() {
  require(&_memory_);
  require(&_universe_);

  Task* rootTask = poolAlloc(&taskPool);
  Task* scheduleTask = poolAlloc(&taskPool);
  
  rr_root.next = &rr_root;
  rr_root.prev = &rr_root;
  rr_root.univ = &rootSpace;
  rr_root.task = rootTask;

  rootTask->tss = tss(kernelSpace.pageDir,0,0,0,0,0,0);
  rootTask->rr = &rr_root;

  scheduleTask->tss = tss(kernelSpace.pageDir,DATA_SEGMENT,INT_STACK,0,0,0,0);
  scheduleTask->tss.eip	= schedule;
  scheduleTask->tss.ss	= DATA_SEGMENT;
  scheduleTask->tss.esp	= INT_STACK;

  scheduleGate = addDesc(&gdt,tssDesc(scheduleTask,0,0));
  slaveGate = addDesc(&gdt,tssDesc(rootTask,0,0));
  flushGDT();

  setTaskRegister(slaveGate);
}
Feature _schedule_ = {
  .state = DISABLED,
  .label = "schedule",
  .initialize = &initSchedule
};

void yield() {
  Descriptor* taskD = &DESCRIPTOR_AT(gdt,getTask()->tss.previousTask);
  Task* task = descBase(*taskD);
  RR* rr = task->rr;

  if(rr != &rr_root) {
    /* detach current timeline */
    rr->prev->next = rr->next;
    rr->next->prev = rr->prev;

    /* reattach it last on the round robin */
    rr->next = &rr_root;
    rr->prev = rr_root.prev;
    rr->prev->next = rr;
    rr->next->prev = rr;
  }

  RR* curRR = rr_root.next;

  /* printf("Yield %x to %x (eip %x, ss %x, esp %x, cr3 %x)\n",rr->task,curRR->task, */
  /* 	 curRR->task->tss.eip,curRR->task->tss.ss,curRR->task->tss.esp, */
  /* 	 curRR->task->tss.cr3); */

  *taskD = tssDesc(curRR->task,curRR->univ->dpl,1);
  flushGDT();
}
void schedule() {
  while(1) {
    millis += phase;
    if(millis >= FREQUENCY) {
      millis -= FREQUENCY;
      seconds++;
      /* printf("Second %d\n",seconds); */
    }
    yield();
    
    outportb(0x20, 0x20);
    asm __volatile__ ( "iret" );
  }
}
Task* newTask() {
  return poolAlloc(&taskPool);
}
Task* getTask() {
  return descBase(DESCRIPTOR_AT(gdt,getTaskRegister()));
}

int findNextSlot() {
  while(stackSlots[curSlot]) {
    curSlot++;
    if(curSlot == MAX_THREADS)
      curSlot = 0;
  }
  stackSlots[curSlot] = 1;

  return curSlot;
}

/*
  Spawns a new timeline on the current universe
  where eip = eax.
 */
void sys_spawn(Task* task) {
  dword eip = task->tss.ebx;

  Universe* univ = task->rr->univ;
  int slot = findNextSlot();
  dword vaddr = STACK_START(slot);
    
  if(univ != &kernelSpace) {
    int i;
    for(i=0;i<STACK_PAGES;i++) {
      void* pg = allocatePage();
      mapPage(univ,STACK_PAGE(vaddr,i),pg);
    }
  }

  Task* s = poolAlloc(&taskPool);
  RR* rr = poolAlloc(&rrPool);

  s->tss = task->tss;
  s->tss.eip = eip;
  s->tss.esp = vaddr;
  s->rr = rr;

  rr->next = rr_root.next;
  rr_root.next = rr;
  rr->prev = &rr_root;
  rr->task = s;
  rr->slot = slot;

  printf("Spawning : tss=%x new=%x\n",task,s);
  
  task->tss.eax = rr;
}
/*
  Ends the current timeline.
 */
void sys_die(Task* task) {
  RR* rr = task->rr;

  rr->next->prev = rr->prev;
  rr->prev->next = rr->next;

  int i; dword start = STACK_START(rr->slot);
  for(i=0;i<STACK_PAGES;i++)
    mapPage(rr->univ,STACK_PAGE(start,i),0);

  stackSlots[rr->slot] = 0;

  poolFree(&taskPool,rr->task);
  poolFree(&rrPool,rr);
}
/*
  Warps the current timeline to another universe,
  where universe descriptor = eax
    and eip = ebx
 */
void sys_warp(Task* task) {
  Universe* u = task->tss.ebx;
  dword eip = task->tss.ecx;

  RR* rr = task->rr;
  dword start = STACK_START(rr->slot);

  int i;
  for(i=0;i<STACK_PAGES;i++) {
    dword vaddr = STACK_PAGE(start,i);
    dword* page = dirVal(u->pageDir,vaddr);
    mapPage(u,vaddr,*page);
    mapPage(rr->univ,vaddr,0);
  }

  task->tss.eip = eip;
  task->tss.esp = start;
  task->tss.cr3 = u->pageDir;
}
