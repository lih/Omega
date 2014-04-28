#include "constants.h"
#include "memory.h"
#include "descriptors.h"
#include "schedule.h"
#include "universe.h"
#include "framebuffer.h"
#include "feature.h"

#define MAX_THREADS 1024
#define STACK_PAGES 16
#define STACK_SIZE  (STACK_PAGES * PAGE_SIZE)
#define STACK_START(slot) (-((1+(slot))*STACK_SIZE))
#define STACK_PAGE(s,i) (s-(i+1)*PAGE_SIZE)
#define FREQUENCY 1193180

Pool taskPool;
Pool rrPool;

RR rr_root;
int millis = 0;
int seconds = 0;
int timerPhase;

byte stackSlots[MAX_THREADS] = { 0 };
int  curSlot = 0;
Selector rootGate,slaveGate;

Universe rootSpace;

void schedule();
void yield();
static void spawn(Task*);
static void die(Task*);
static void warp(Task*);

void init() {
  printf("Text from another process !\n");
  syscall_die();
}
static void initSchedule() {
  require(&_memory_);
  require(&_universe_);
  require(&_interrupts_);

  taskPool = pool(sizeof(Task));
  rrPool = pool(sizeof(RR));

  Task* rootTask = poolAlloc(&taskPool);
  
  rr_root.next = &rr_root;
  rr_root.prev = &rr_root;
  rr_root.univ = &rootSpace;
  rr_root.task = rootTask;

  SET_STRUCT(Universe,rootSpace,{
      .pageDir = newDir(),
	.dpl = 0
	});
  
  /* Identity map the stack and code pages */
  mapPage(&rootSpace,STACK_PAGE(KERNEL_STACK,0),STACK_PAGE(KERNEL_STACK,0));
  void* kpage = KERNEL_START & 0xfffff000;
  for(;kpage < KERNEL_START+KERNEL_SIZE;kpage+=PAGE_SIZE)
    mapPage(&rootSpace,kpage,kpage);
  for(kpage = FB_MEM & 0xfffff000;kpage < FB_END;kpage+=PAGE_SIZE)
    mapPage(&rootSpace,kpage,kpage);

  
  rootTask->tss = tss(rootSpace.pageDir,0,0,0,0,0,0);
  rootTask->rr = &rr_root;

  Task* mgmtTask = poolAlloc(&taskPool);

  mgmtTask->tss = tss(kernelSpace.pageDir,DATA_SEGMENT,INT_STACK,0,0,0,0);
  mgmtTask->tss.eip	= schedule;
  mgmtTask->tss.esp	= INT_STACK;
  mgmtTask->tss.ss	= DATA_SEGMENT;

  rootGate = addDesc(&gdt,tssDesc(mgmtTask,0,1));
  slaveGate = addDesc(&gdt,tssDesc(rootTask,0,0));
  flushGDT();

  idts[32] = taskGate(rootGate,0);
  syscalls[SYS_WARP] = warp;
  syscalls[SYS_SPAWN] = spawn;
  syscalls[SYS_DIE] = die;

  setTimerFreq(TIMER_FREQ);
  padLine(); printf("Set timer frequency to %dHz\n",TIMER_FREQ);
  
  setPageDirectory(rootSpace.pageDir);
  printf("Changed universe.\n");
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

  printf("Yield %x to %x (eip %x, ss %x, esp %x, cr3 %x)\n",rr->task,curRR->task,
	 curRR->task->tss.eip,curRR->task->tss.ss,curRR->task->tss.esp,
	 curRR->task->tss.cr3);

  *taskD = tssDesc(curRR->task,curRR->univ->dpl,1);
  flushGDT();

  int i; for(i=0;i<10000000;i++);
}
void schedule() {
  while(1) {
    millis += timerPhase;
    if(millis >= FREQUENCY) {
      millis -= FREQUENCY;
      seconds++;
      printf("Second %d\n",seconds);
    }
    yield();
    
    outportb(0x20, 0x20);
    asm __volatile__ ( "iret" );
  }
}
void setTimerFreq(int hz) {
  timerPhase = FREQUENCY / hz;       
  outportb(0x43, 0x36);           /* Set our command byte 0x36 */
  outportb(0x40, timerPhase & 0xff);
  outportb(0x40, timerPhase >> 8); /* Set two-byte phase */
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
void spawn(Task* task) {
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
void die(Task* task) {
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
void warp(Task* task) {
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

