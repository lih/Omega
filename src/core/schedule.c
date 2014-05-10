#include <constants.h>
#include <cpu/memory.h>
#include <cpu/descriptors.h>
#include <core/schedule.h>
#include <core/universe.h>
#include <device/framebuffer.h>
#include <core/feature.h>
#include <device/timer.h>

Pool taskPool = { NULL, sizeof(Task) };

byte stackSlots[MAX_THREADS] = { 0 };
int  curSlot = 0;
Selector scheduleGate,slaveGate;

Dir* taskDirectory;
  
Task activeRoot = {
  .next = &activeRoot,
  .prev = &activeRoot,
  .univ = &kernelSpace,
  .tss = KERNEL_STACK,
  .slot = 0
};
Task pendingRoot = {
  .next = &pendingRoot,
  .prev = &pendingRoot
};
Task waitingRoot = {
  .next = &waitingRoot,
  .prev = &waitingRoot
};

void schedule();

static void initSchedule() {
  require(&_memory_);
  require(&_universe_);

  taskDirectory = newDir();
  *dirVal(taskDirectory,activeRoot.tss) = &activeRoot;
  slaveGate = addDesc(&gdt,tssDesc(activeRoot.tss,0));
  *(activeRoot.tss) = tss(kernelSpace.pageDir,0,0);

  Task* scheduleTask = poolAlloc(&taskPool);

  scheduleTask->tss = INT_STACK - sizeof(TSS);
  *(scheduleTask->tss) = tss(kernelSpace.pageDir,schedule,INT_STACK - sizeof(TSS));
  scheduleGate = addDesc(&gdt,tssDesc(scheduleTask->tss,0));
  
  flushGDT();

  setTaskRegister(slaveGate);
}
Feature _schedule_ = {
  .state = DISABLED,
  .label = "schedule",
  .initialize = &initSchedule
};

Task* getTask(Selector gate) {
  TSS* tss = descBase(DESCRIPTOR_AT(gdt,gate));
  Task** task = dirVal(taskDirectory,tss);
  
  return *task;
}
void detachTask(Task* t) {
  t->next->prev = t->prev;
  t->prev->next = t->next;
}
void insertTask(Task* a,Task* b) {
  a->prev = b->prev;
  a->next = b;
  a->next->prev = a;
  a->prev->next = a;
}
void scheduleNext(TSS* cur) {
  DESCRIPTOR_AT(gdt,cur->previousTask) = tssDesc(activeRoot.next->tss,1);
  flushGDT();
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
    Task* cur = waitingRoot.next;
    Task* next;
    while(cur != &waitingRoot) {
      next = cur->next;
      if(cur->trig.time.seconds < seconds || 
	 (cur->trig.time.seconds == seconds && cur->trig.time.millis < millis)) {
	detachTask(cur);
	insertTask(cur,&activeRoot);
      }
      cur = next;
    }

    if(activeRoot.next != &activeRoot) {
      Task* t = activeRoot.next;
      detachTask(t);
      insertTask(t,&activeRoot);
    }

    scheduleNext(ss);

    /* printf("Schedule at eip=%x esp=%x tss=%x cr3=%x\n" */
    /* 	   ,curTask->tss->eip,curTask->tss->esp,curTask->tss,curTask->tss->cr3); */

    outportb(0x20, 0x20);
    asm __volatile__ ( "iret" );
  }
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
Universe* univAt(Task* t,int index) {
  switch(index) {
  case -2:
    return t->from;
  case -1:
    return t->univ;
  }

  Universe* end = t->univ->down;
  Universe* cur = end->right;
  while(cur != end) {
    if(cur->index == index)
      return cur;
    cur = cur->right;
  }

  printf("Unknown universe %d\n",index);
  return NULL;
}

void releaseSem(Semaphore* paddr,int n) {
  Task* curTask = pendingRoot.next;
  while(curTask != &pendingRoot) {
    if(curTask->trig.sem == paddr) {
      Task* next = curTask->next;
      detachTask(curTask);
      insertTask(curTask,&activeRoot);
      curTask->tss->eax = *paddr + n;
      n--;
      if(n == 0)
	break;
      curTask = next;
    }
    else
      curTask = curTask->next;
  }
  *paddr += n;
}

void sys_spark(Task* task) {
  Universe* univ = univAt(task,task->tss->ebx);
  dword eip = task->tss->ecx;

  int slot = findNextSlot();
  dword vaddr = STACK_START(slot);
    
  int i;
  for(i=0;i<STACK_PAGES;i++) {
    void* pg = allocatePage();
    mapPage(univ,STACK_PAGE(vaddr,i),pg,1);
    mapPage(&kernelSpace,STACK_PAGE(vaddr,i),pg,1);
  }
  
  Task* newtask = poolAlloc(&taskPool); {
    newtask->tss = vaddr-sizeof(TSS);
    *(newtask->tss) = tss(univ->pageDir,eip,vaddr-sizeof(TSS));
    newtask->tss->eflags._if = 1;
    newtask->tss->eflags.iopl = univ->dpl;
    *dirVal(taskDirectory,newtask->tss) = newtask;

    insertTask(newtask,&activeRoot);
    newtask->slot = slot;
    newtask->univ = univ;
  }
}
void sys_die(Task* task) {
  detachTask(task);

  int i; dword start = STACK_START(task->slot);
  for(i=0;i<STACK_PAGES;i++)
    mapPage(task->univ,STACK_PAGE(start,i),0,0);

  stackSlots[task->slot] = 0;

  poolFree(&taskPool,task);
}
void sys_warp(Task* task) {
  Universe* u = univAt(task,task->tss->ebx);
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
  task->tss->eflags.iopl = u->dpl;
  task->from = task->univ;
  task->univ = u;
}
void sys_acquire(Task* task) {
  dword vaddr = task->tss->ebx;

  DirEntry* d = dirVal(task->univ->pageDir,vaddr);
  Semaphore* paddr = ((dword)F_ADDR(*d)) | (vaddr & 0xfff);

  if(*paddr == 0) {
    task->trig.sem = paddr;
  
    detachTask(task);
    insertTask(task,pendingRoot.next);
  }
  else
    (*paddr)--;
}

void sys_release(Task* task) {
  dword vaddr = task->tss->ebx;
  int n = task->tss->ecx;

  DirEntry* d = dirVal(task->univ->pageDir,vaddr);
  Semaphore* paddr = ((void*)F_ADDR(*d)) + (vaddr & 0xfff);
  
  releaseSem(paddr,n);
}
void sys_mapTo(Task* task) {
  Universe* dest = univAt(task,task->tss->ebx);
  dword from = task->tss->ecx;
  dword to = task->tss->edx;

  if(!IS(dest,NULL)) {
    DirEntry* d = dirVal(task->univ->pageDir,from);
    void* paddr = F_ADDR(*d);
  
    mapPage(dest,to,paddr,1);
  }
}
void sys_mapFrom(Task* task) {
  Universe* dest = univAt(task,task->tss->ebx);
  dword from = task->tss->ecx;
  dword to = task->tss->edx;

  if(!IS(dest,NULL)) {
    DirEntry* d = dirVal(dest->pageDir,from);
    void* paddr = F_ADDR(*d);
    
    mapPage(task->univ,to,paddr,1);
  }
}
void sys_spawn(Task* t) {
  Universe* u = newUniverse(t->univ);

  t->tss->eax = u->index;
}
void sys_anihilate(Task* t) {
  Universe* u = univAt(t,t->tss->ebx);

  freeUniverse(u);
}
void sys_wait(Task* task) {
  int secs = task->tss->ebx,
    ms = task->tss->ecx;
  
  detachTask(task);
  insertTask(task,&waitingRoot);

  task->trig.time.seconds = secs + seconds;
  task->trig.time.millis = millis + (ms*FREQUENCY)/1000;
  if(task->trig.time.millis >= FREQUENCY) {
    task->trig.time.seconds ++;
    task->trig.time.millis -= FREQUENCY;
  }
}
void sys_alloc(struct Task* t) {
  dword vpage = t->tss->ebx;
  int n = t->tss->ecx;

  int i;
  for(i=0;i<n;i++)
    mapPage(t->univ,vpage+(i*PAGE_SIZE),allocatePage(),1);
}
