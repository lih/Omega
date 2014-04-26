#include "constants.h"
#include "memory.h"
#include "descriptors.h"
#include "schedule.h"
#include "universe.h"
#include "framebuffer.h"

#define MAX_THREADS 1024
#define STACK_PAGES 16
#define STACK_SIZE  (STACK_PAGES * PAGE_SIZE)
#define STACK_START(slot) (-((1+(slot))*STACK_SIZE))
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

void timerIRQ(IDTParams*);

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
TSS* getTSS();

void initSchedule() {
  tssPool = pool(sizeof(TSS));
  rrPool = pool(sizeof(RR));
  
  TSS* rootTSS = poolAlloc(&tssPool);
  *rootTSS = tss(kernelSpace.pageDir,DATA_SEGMENT,KERNEL_STACK,0,0,0,0);
  rootTSS->rr = &rr_root;

  rr_root.next = &rr_root;
  rr_root.prev = &rr_root;
  rr_root.tss = rootTSS;
  
  rootGate = addDescriptor(tssDesc(rootTSS,0));
  slaveGate = addDescriptor(tssDesc(rootTSS,0));

  setTaskRegister(slaveGate);

  irqs[0] = timerIRQ;
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
void yield() {
  TSS* tss = getTSS();

  RR* rr = tss->rr;
  
  /* detach current timeline */
  rr->prev->next = rr->next;
  rr->next->prev = rr->prev;

  /* reattach it last on the round robin */
  rr->next = &rr_root;
  rr->prev = rr_root.prev;
  rr_root.prev = rr;
  
  RR* curRR = rr_root.next;

  tss->previousTask = slaveGate;
  gdtDesc.base[slaveGate / sizeof(Descriptor)] = tssDesc(curRR->tss,curRR->univ->dpl);
  flushGDT();
}
RR* spawn(dword eip) {
  TSS* curtss = getTSS();
  Universe* univ = curtss->rr->univ;
  int slot = findNextSlot();
  dword vaddr = STACK_START(slot);
    
  if(univ != &kernelSpace) {
    int i;
    for(i=0;i<STACK_PAGES;i++) {
      void* pg = allocatePage();
      mapPage(univ,vaddr+i*PAGE_SIZE,pg);
    }
  }

  TSS* s = poolAlloc(&tssPool);

  *s = *curtss;
  s->eip = eip;
  s->esp = vaddr;

  RR* rr = poolAlloc(&rrPool);
    
  rr->next = rr_root.next;
  rr_root.next = rr;
  rr->prev = &rr_root;
  rr->tss = s;
  rr->slot = slot;
  
  s->rr = rr;
  
  return rr;
}
void die() {
  RR* rr = getTSS()->rr;

  rr->next->prev = rr->prev;
  rr->prev->next = rr->next;

  int i; dword start = STACK_START(rr->slot);
  for(i=0;i<STACK_PAGES;i++)
    mapPage(rr->univ,start+i*PAGE_SIZE,0);

  stackSlots[rr->slot] = 0;

  poolFree(&tssPool,rr->tss);
  poolFree(&rrPool,rr);
}

TSS* getTSS() {
  word tr = getTaskRegister();
  return descBase(gdtDesc.base[tr / sizeof(Descriptor)]);
}
void timerIRQ(IDTParams* _) {
  millis += timerPhase;
  if(millis >= FREQUENCY) {
    millis -= FREQUENCY;
    seconds++;
    /* printf("Second %d\n",seconds); */
  }
  yield();
}
void setTimerFreq(int hz) {
  timerPhase = FREQUENCY / hz;       
  outportb(0x43, 0x36);           /* Set our command byte 0x36 */
  outportb(0x40, timerPhase & 0xff);
  outportb(0x40, timerPhase >> 8); /* Set two-byte phase */
}

