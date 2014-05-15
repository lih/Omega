#include <constants.h>
#include <util/memory.h>
#include <x86/descriptors.h>
#include <core/life.h>
#include <core/universe.h>
#include <device/framebuffer.h>
#include <util/feature.h>
#include <device/timer.h>
#include <x86/pervasives.h>

Pool taskPool = { NULL, sizeof(Life) };

byte stackSlots[MAX_THREADS] = { 0 };
int  curSlot = 0;
Selector scheduleGate,slaveGate;

Dir* taskDirectory;
  
Life activeRoot = {
  .next = &activeRoot,
  .prev = &activeRoot,
  .univ = &kernelSpace,
  .tss = (TSS*)KERNEL_STACK,
  .slot = 0
};
Life pendingRoot = {
  .next = &pendingRoot,
  .prev = &pendingRoot
};
Life waitingRoot = {
  .next = &waitingRoot,
  .prev = &waitingRoot
};

void schedule();

Dir* newDir();

static void initSchedule() {
  require(&_memory_);
  require(&_universe_);

  taskDirectory = newDir();
  *dirVal(taskDirectory,activeRoot.tss) = (DirEntry)&activeRoot;
  slaveGate = addDesc(&gdt,tssDesc(activeRoot.tss,0));
  *(activeRoot.tss) = tss(kernelSpace.pageDir,0,0);

  Life* scheduleLife = kPoolAlloc(&taskPool);

  scheduleLife->tss = (void*)INT_STACK - sizeof(TSS);
  *(scheduleLife->tss) = tss(kernelSpace.pageDir,schedule,scheduleLife->tss);
  scheduleGate = addDesc(&gdt,tssDesc(scheduleLife->tss,0));
  
  flushGDT();

  setLifeRegister(slaveGate);
}
Feature _schedule_ = {
  .state = DISABLED,
  .label = "schedule",
  .initialize = &initSchedule
};

Life* getLife(Selector gate) {
  TSS* tss = descBase(DESCRIPTOR_AT(gdt,gate));
  Life** task = (Life**)dirVal(taskDirectory,tss);
  
  return *task;
}
void detachLife(Life* t) {
  t->next->prev = t->prev;
  t->prev->next = t->next;
}
void insertLife(Life* a,Life* b) {
  a->prev = b->prev;
  a->next = b;
  a->next->prev = a;
  a->prev->next = a;
}
void scheduleNext(TSS* cur) {
  DESCRIPTOR_AT(gdt,cur->previousLife) = tssDesc(activeRoot.next->tss,1);
  flushGDT();
}
void schedule() {
  require(&_schedule_);
  TSS* ss = TSS_AT(getLifeRegister());

  while(1) {
    millis += phase;
    if(millis >= FREQUENCY) {
      millis -= FREQUENCY;
      seconds++;
      /* printf("Second %d\n",seconds); */
    }
    Life* cur = waitingRoot.next;
    Life* next;
    while(cur != &waitingRoot) {
      next = cur->next;
      if(cur->trig.time.seconds < seconds || 
	 (cur->trig.time.seconds == seconds && cur->trig.time.millis < millis)) {
	detachLife(cur);
	insertLife(cur,&activeRoot);
      }
      cur = next;
    }

    if(activeRoot.next != &activeRoot) {
      Life* t = activeRoot.next;
      detachLife(t);
      insertLife(t,&activeRoot);
    }

    scheduleNext(ss);

    /* printf("Schedule at eip=%x esp=%x tss=%x cr3=%x\n" */
    /* 	   ,curLife->tss->eip,curLife->tss->esp,curLife->tss,curLife->tss->cr3); */

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
Universe* univAt(Life* t,int index) {
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
  Life* curLife = pendingRoot.next;
  while(curLife != &pendingRoot) {
    if(curLife->trig.sem == paddr) {
      Life* next = curLife->next;
      detachLife(curLife);
      insertLife(curLife,&activeRoot);
      curLife->tss->eax = *paddr + n;
      n--;
      if(n == 0)
	break;
      curLife = next;
    }
    else
      curLife = curLife->next;
  }
  *paddr += n;
}

void sys_spark(Life* task) {
  Universe* univ = univAt(task,task->tss->ebx);
  void* eip = (void*)task->tss->ecx;

  int slot = findNextSlot();
  dword vaddr = STACK_START(slot);
    
  int i;
  for(i=0;i<STACK_PAGES;i++) {
    void* pg = kAllocPage();
    mapPage(univ,STACK_PAGE(vaddr,i),pg,1);
    mapPage(&kernelSpace,STACK_PAGE(vaddr,i),pg,1);
  }
  
  Life* newtask = kPoolAlloc(&taskPool); {
    newtask->tss = (void*)vaddr-sizeof(TSS);
    *(newtask->tss) = tss(univ->pageDir,eip,newtask->tss);
    newtask->tss->eflags._if = 1;
    newtask->tss->eflags.iopl = univ->dpl;
    *dirVal(taskDirectory,newtask->tss) = (DirEntry)newtask;

    insertLife(newtask,&activeRoot);
    newtask->slot = slot;
    newtask->univ = univ;
  }
}
void sys_die(Life* task) {
  detachLife(task);

  int i; dword start = STACK_START(task->slot);
  for(i=0;i<STACK_PAGES;i++)
    mapPage(task->univ,STACK_PAGE(start,i),0,0);

  stackSlots[task->slot] = 0;

  kPoolFree(&taskPool,task);
}
void sys_warp(Life* task) {
  Universe* u = univAt(task,task->tss->ebx);
  dword eip = task->tss->ecx;
  dword start = STACK_START(task->slot);

  int i;
  for(i=0;i<STACK_PAGES;i++) {
    void* vaddr = STACK_PAGE(start,i);
    void** page = (void**)dirVal(task->univ->pageDir,vaddr);
    mapPage(u,vaddr,*page,1);
    mapPage(task->univ,vaddr,0,0);
  }

  task->tss->eip = eip;
  task->tss->esp = start;
  task->tss->cr3 = (dword)u->pageDir;
  task->tss->eflags.iopl = u->dpl;
  task->from = task->univ;
  task->univ = u;
}
void sys_acquire(Life* task) {
  void* vaddr = (void*)task->tss->ebx;

  DirEntry* d = dirVal(task->univ->pageDir,vaddr);
  Semaphore* paddr = (Semaphore*)(((dword)F_ADDR(*d)) | ((dword)vaddr & 0xfff));

  if(*paddr == 0) {
    task->trig.sem = paddr;
  
    detachLife(task);
    insertLife(task,pendingRoot.next);
  }
  else
    (*paddr)--;
}

void sys_release(Life* task) {
  void* vaddr = (void*)task->tss->ebx;
  int n = task->tss->ecx;

  DirEntry* d = dirVal(task->univ->pageDir,vaddr);
  Semaphore* paddr = (Semaphore*)(((void*)F_ADDR(*d)) + ((dword)vaddr & 0xfff));
  
  releaseSem(paddr,n);
}
void sys_mapTo(Life* task) {
  Universe* dest = univAt(task,task->tss->ebx);
  void* from = (void*)task->tss->ecx;
  void* to = (void*)task->tss->edx;

  if(!IS(dest,NULL)) {
    DirEntry* d = dirVal(task->univ->pageDir,from);
    void* paddr = F_ADDR(*d);
  
    mapPage(dest,to,paddr,1);
  }
}
void sys_mapFrom(Life* task) {
  Universe* dest = univAt(task,task->tss->ebx);
  void* from = (void*)task->tss->ecx;
  void* to = (void*)task->tss->edx;

  if(!IS(dest,NULL)) {
    DirEntry* d = dirVal(dest->pageDir,from);
    void* paddr = F_ADDR(*d);
    
    mapPage(task->univ,to,paddr,1);
  }
}
void sys_spawn(Life* t) {
  Universe* u = newUniverse(t->univ);

  t->tss->eax = u->index;
}
void sys_anihilate(Life* t) {
  Universe* u = univAt(t,t->tss->ebx);

  freeUniverse(u);
}
void sys_wait(Life* task) {
  int secs = task->tss->ebx,
    ms = task->tss->ecx;
  
  detachLife(task);
  insertLife(task,&waitingRoot);

  task->trig.time.seconds = secs + seconds;
  task->trig.time.millis = millis + (ms*FREQUENCY)/1000;
  if(task->trig.time.millis >= FREQUENCY) {
    task->trig.time.seconds ++;
    task->trig.time.millis -= FREQUENCY;
  }
}
void sys_alloc(struct Life* t) {
  void* vpage = (void*)t->tss->ebx;
  int n = t->tss->ecx;

  int i;
  for(i=0;i<n;i++)
    mapPage(t->univ,vpage+(i*PAGE_SIZE),kAllocPage(),1);
}
