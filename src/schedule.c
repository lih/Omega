#include "constants.h"
#include "memory.h"
#include "descriptors.h"
#include "schedule.h"
#include "universe.h"
#include "framebuffer.h"
#include "feature.h"
#include "timer.h"

Pool taskPool = { NULL, sizeof(Task) };

byte stackSlots[MAX_THREADS] = { 0 };
int  curSlot = 0;
Selector scheduleGate,slaveGate;

Dir* taskDirectory;
  
Universe rootSpace;
Task task_root = {
  .next = &task_root,
  .prev = &task_root,
  .univ = &rootSpace,
  .tss = KERNEL_STACK,
  .slot = 0
};

void schedule();
void yield();

static void initSchedule() {
  require(&_memory_);
  require(&_universe_);

  taskDirectory = newDir();
  *dirVal(taskDirectory,task_root.tss) = &task_root;
  slaveGate = addDesc(&gdt,tssDesc(task_root.tss,0,0));
  *(task_root.tss) = tss(kernelSpace.pageDir,0,0);

  Task* scheduleTask = poolAlloc(&taskPool);

  scheduleTask->tss = INT_STACK - sizeof(TSS);
  *(scheduleTask->tss) = tss(kernelSpace.pageDir,schedule,INT_STACK - sizeof(TSS));
  scheduleGate = addDesc(&gdt,tssDesc(scheduleTask->tss,0,0));
  
  flushGDT();

  setTaskRegister(slaveGate);
}
Feature _schedule_ = {
  .state = DISABLED,
  .label = "schedule",
  .initialize = &initSchedule
};

void yield(Task* task) {
  if(task != &task_root) {
    /* detach current timeline */
    task->prev->next = task->next;
    task->next->prev = task->prev;

    /* reattach it last on the round robin */
    task->next = &task_root;
    task->prev = task_root.prev;
    task->prev->next = task;
    task->next->prev = task;
  }
}
void schedule() {
  require(&_schedule_);
  TSS* ss = TSS_AT(getTaskRegister());

  while(1) {
    millis += phase;
    if(millis >= FREQUENCY) {
      millis -= FREQUENCY;
      seconds++;
      /* printf("Second %d\n",seconds); */
    }

    Task* t = getTask(ss->previousTask);
    if(!IS(t,NULL)) yield(t);
    
    Task* curTask = task_root.next;
    DESCRIPTOR_AT(gdt,ss->previousTask) = tssDesc(curTask->tss,curTask->univ->dpl,1);
    flushGDT();

    /* printf("Schedule at eip=%x esp=%x tss=%x cr3=%x\n" */
    /* 	   ,curTask->tss->eip,curTask->tss->esp,curTask->tss,curTask->tss->cr3); */

    outportb(0x20, 0x20);
    asm __volatile__ ( "iret" );
  }
}
Task* getTask(Selector gate) {
  TSS* tss = descBase(DESCRIPTOR_AT(gdt,gate));
  Task** task = dirVal(taskDirectory,tss);
  
  return *task;
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
  Universe* univ = task->tss->ebx;
  dword eip = task->tss->ecx;

  int slot = findNextSlot();
  dword vaddr = STACK_START(slot);
    
  int i;
  for(i=0;i<STACK_PAGES;i++) {
    void* pg = allocatePage();
    mapPage(univ,STACK_PAGE(vaddr,i),pg,1);
    mapPage(&kernelSpace,STACK_PAGE(vaddr,i),pg,0);
  }
  
  Task* newtask = poolAlloc(&taskPool); {
    newtask->tss = vaddr-sizeof(TSS);
    *(newtask->tss) = tss(univ->pageDir,eip,vaddr-sizeof(TSS));
    newtask->tss->eflags.iopl = 1;
    *dirVal(taskDirectory,newtask->tss) = newtask;

    newtask->next = task_root.next;
    task_root.next = newtask;
    newtask->prev = &task_root;
    newtask->slot = slot;
    newtask->univ = univ;
  }

  task->tss->eax = newtask;
}
/*
  Ends the current timeline.
 */
void sys_die(Task* task) {
  task->next->prev = task->prev;
  task->prev->next = task->next;

  int i; dword start = STACK_START(task->slot);
  for(i=0;i<STACK_PAGES;i++)
    mapPage(task->univ,STACK_PAGE(start,i),0,0);

  stackSlots[task->slot] = 0;

  poolFree(&taskPool,task);
  poolFree(&taskPool,task);
}
/*
  Warps the current timeline to another universe,
  where universe descriptor = ebx
    and eip = ecx
 */
void sys_warp(Task* task) {
  Universe* u = task->tss->ebx;
  dword eip = task->tss->ecx;
  dword start = STACK_START(task->slot);

  int i;
  for(i=0;i<STACK_PAGES;i++) {
    dword vaddr = STACK_PAGE(start,i);
    dword* page = dirVal(task->univ->pageDir,vaddr);
    mapPage(u,vaddr,*page,1);
    mapPage(task->univ,vaddr,0,0);
  }

  task->tss->eip = eip;
  task->tss->esp = start;
  task->tss->cr3 = u->pageDir;
  task->univ = u;
}
