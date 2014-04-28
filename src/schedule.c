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

Pool tssPool;
Pool rrPool;

RR rr_root;
int millis = 0;
int seconds = 0;
int timerPhase;

byte stackSlots[MAX_THREADS] = { 0 };
int  curSlot = 0;
word rootGate,slaveGate;
Universe rootSpace;

void schedule();
void yield();
static void spawn(TSS*);
static void die(TSS*);
static void warp(TSS*);

void init() {
  printf("Text from another process !\n");
  syscall_die();
}
void initSchedule() {
  require(MEMORY);
  require(UNIVERSE);
  require(INTERRUPTS);

  tssPool = pool(sizeof(TSS));
  rrPool = pool(sizeof(RR));

  TSS* rootTSS = poolAlloc(&tssPool);
  
  rr_root.next = &rr_root;
  rr_root.prev = &rr_root;
  rr_root.univ = &rootSpace;
  rr_root.tss = rootTSS;

  SET_STRUCT(Universe,rootSpace,{
      .pageDir = newDir(),
	.dpl = 0
	});
  
  /* Identity map the stack and code pages */
  mapPage(&rootSpace,STACK_PAGE(KERNEL_STACK,0),STACK_PAGE(KERNEL_STACK,0));
  void* kpage = KERNEL_START & 0xfffff000;
  for(;kpage < KERNEL_START+KERNEL_SIZE;kpage+=PAGE_SIZE)
    mapPage(&rootSpace,kpage,kpage);
  
  *rootTSS = tss(rootSpace.pageDir,0,0,0,0,0,0);
  rootTSS->rr = &rr_root;

  TSS* mgmtTSS = poolAlloc(&tssPool);

  *mgmtTSS = tss(kernelSpace.pageDir,DATA_SEGMENT,INT_STACK,0,0,0,0);
  mgmtTSS->eip = schedule;
  mgmtTSS->esp = INT_STACK;
  mgmtTSS->ss = DATA_SEGMENT;

  rootGate = addDescriptor(tssDesc(mgmtTSS,0));
  slaveGate = addDescriptor(tssDesc(rootTSS,0));

  idts[32] = tssGate(rootGate,0);
  syscalls[SYS_WARP] = warp;
  syscalls[SYS_SPAWN] = spawn;
  syscalls[SYS_DIE] = die;

  setTimerFreq(TIMER_FREQ);
  padLine(); printf("Set timer frequency to %dHz\n",TIMER_FREQ);
  
  setTaskRegister(slaveGate);
}

void yield() {
  int taskInd = getTSS()->previousTask / sizeof(Descriptor);
  TSS* tss = descBase(gdtDesc.base[taskInd]);
  RR* rr = tss->rr;

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

  printf("Yield %x to %x (eip %x, ss %x, esp %x, cr3 %x)\n",rr->tss,curRR->tss,
	 curRR->tss->eip,curRR->tss->ss,curRR->tss->esp,curRR->tss->cr3);

  gdtDesc.base[taskInd] = tssDesc(curRR->tss,curRR->univ->dpl);
  gdtDesc.base[taskInd].type |= 2;
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

Descriptor tssDesc(TSS* data,byte dpl) {
  dword base = data;

  Descriptor ret = {
    .limit_lo = sizeof(TSS),
    .base_lo = base,
    .base_mi = base >> 16,
    .type = 9,
    .codeOrData = 0,
    .dpl = dpl,
    .present = 1,
    .limit_hi = 0,
    .code64 = 0,
    .opsize = 1,
    .granularity = 1,
    .base_hi = base >> 24
  };

  return ret;
}
TSS        tss(Dir* pageDir,word ss0,dword esp0,
	       word ss1,dword esp1,word ss2,dword esp2) {
  TSS ret = {
    .esp0 = esp0, .ss0 = ss0,
    .esp1 = esp1, .ss1 = ss1,
    .esp2 = esp2, .ss2 = ss2,
    .cs = CODE_SEGMENT, .ds = DATA_SEGMENT, .es = DATA_SEGMENT,
    .fs = DATA_SEGMENT, .gs = DATA_SEGMENT, .ss = DATA_SEGMENT,
    .cr3 = (dword)pageDir,
    .trap = 0
  };

  return ret;
}
TSS* newTSS() {
  return poolAlloc(&tssPool);
}
TSS* getTSS() {
  word tr = getTaskRegister();
  return descBase(gdtDesc.base[tr / sizeof(Descriptor)]);
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
void spawn(TSS* tss) {
  dword eip = tss->ebx;

  Universe* univ = tss->rr->univ;
  int slot = findNextSlot();
  dword vaddr = STACK_START(slot);
    

  if(univ != &kernelSpace) {
    int i;
    for(i=0;i<STACK_PAGES;i++) {
      void* pg = allocatePage();
      mapPage(univ,STACK_PAGE(vaddr,i),pg);
    }
  }

  TSS* s = poolAlloc(&tssPool);

  *s = *tss;
  s->eip = eip;
  s->esp = vaddr;

  printf("Spawning : tss=%x new=%x\n",tss,s);
  
  RR* rr = poolAlloc(&rrPool);
    
  rr->next = rr_root.next;
  rr_root.next = rr;
  rr->prev = &rr_root;
  rr->tss = s;
  rr->slot = slot;
  
  s->rr = rr;
  
  tss->eax = rr;
}
/*
  Ends the current timeline.
 */
void die(TSS* tss) {
  RR* rr = tss->rr;

  rr->next->prev = rr->prev;
  rr->prev->next = rr->next;

  int i; dword start = STACK_START(rr->slot);
  for(i=0;i<STACK_PAGES;i++)
    mapPage(rr->univ,STACK_PAGE(start,i),0);

  stackSlots[rr->slot] = 0;

  poolFree(&tssPool,rr->tss);
  poolFree(&rrPool,rr);
}
/*
  Warps the current timeline to another universe,
  where universe descriptor = eax
    and eip = ebx
 */
void warp(TSS* tss) {
  Universe* u = tss->ebx;
  dword eip = tss->ecx;

  RR* rr = tss->rr;
  dword start = STACK_START(rr->slot);

  int i;
  for(i=0;i<STACK_PAGES;i++) {
    dword vaddr = STACK_PAGE(start,i);
    dword* page = dirVal(u->pageDir,vaddr);
    mapPage(u,vaddr,*page);
    mapPage(rr->univ,vaddr,0);
  }

  tss->eip = eip;
  tss->esp = start;
  tss->cr3 = u->pageDir;
  
}

