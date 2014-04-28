#include "constants.h"
#include "interrupt.h"
#include "framebuffer.h"
#include "pervasives.h"
#include "schedule.h"
#include "feature.h"
#include "syscall.h"
#include "timer.h"

TablePtr idt = {
  .limit = 49*sizeof(Descriptor) - 1,
  .base = &idts
};

void isr0(); void isr1(); void isr2(); void isr3(); void isr4(); void isr5(); void isr6(); void isr7(); void isr8(); void isr9(); void isr10(); void isr11(); void isr12(); void isr13(); void isr14(); void isr15(); void isr16(); void isr17(); void isr18(); 
void irq0(); void irq1(); void irq2(); void irq3(); void irq4(); void irq5(); void irq6(); void irq7(); void irq8(); void irq9(); void irq10(); void irq11(); void irq12(); void irq13(); void irq14(); void irq15(); void irq16(); 
void reservedException();

char* descriptions[32]; 
IDTHandler irqs[16] = { 0 };
SyscallHandler syscalls[16] = { 0 };

static void initInterrupts() {
  require(&_exceptions_);
  require(&_irqs_);
  require(&_syscalls_);
   
  loadInterrupts();

  outportb(0x20, 0x11);
  outportb(0xA0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xA1, 0x28);
  outportb(0x21, 0x04);
  outportb(0xA1, 0x02);
  outportb(0x21, 0x01);
  outportb(0xA1, 0x01);
  outportb(0x21, 0x0);
  outportb(0xA1, 0x0);
    
  enableInterrupts();
}
Feature _interrupts_ = {
  .state = DISABLED,
  .label = "interrupts",
  .initialize = initInterrupts
};

#define DEFIDT(base,off,n) \
  idts[off+n] = interruptGate(CODE_SEGMENT,base##n,0)

#define DEFEXC(n) DEFIDT(isr,0,n)
static void initExceptions() {
  descriptions[0]="Division by Zero";
  descriptions[1]="Debug";
  descriptions[2]="Non Maskable Interrupt";
  descriptions[3]="Breakpoint";
  descriptions[4]="Overflow";
  descriptions[5]="Out of Bounds";
  descriptions[6]="Invalid Opcode";
  descriptions[7]="No Coprocessor";
  descriptions[8]="Double Fault";
  descriptions[9]="Coprocessor Segment Overrun";
  descriptions[10]="Bad TSS";
  descriptions[11]="Segment Not Present";
  descriptions[12]="Stack Fault";
  descriptions[13]="General Protection Fault";
  descriptions[14]="Page Fault";
  descriptions[15]="Unknown Interrupt";
  descriptions[16]="Coprocessor Fault";
  descriptions[17]="Alignment Check";
  descriptions[18]="Machine Check";

  DEFEXC(0);
  DEFEXC(1);
  DEFEXC(2);
  DEFEXC(3);
  DEFEXC(4);
  DEFEXC(5);
  DEFEXC(6);
  DEFEXC(7);
  DEFEXC(8);
  DEFEXC(9);
  DEFEXC(10);
  DEFEXC(11);
  DEFEXC(12);
  DEFEXC(13);
  DEFEXC(14);
  DEFEXC(15);
  DEFEXC(16);
  DEFEXC(17);
  DEFEXC(18);
  
  int i;
  for(i=19;i<32;i++) {
    idts[i] = interruptGate(CODE_SEGMENT,(dword)reservedException,0);
    descriptions[i]="Reserved";
  }
}
Feature _exceptions_ = {
  .state = DISABLED,
  .label = "exceptions",
  .initialize = &initExceptions
};

void handleException(IDTParams* regs) {
  dword ex_num = regs->int_no;

  if(ex_num < 32) {
    printf("Caught Exception %d (%s) with error code %d\n",ex_num,descriptions[ex_num],regs->err_code);
    printf("eax=%x, esp=%x, eip=%x, cs=%x\n",regs->eax,regs->useresp,regs->eip,regs->cs);
    while(1) {
      nop();
    }
  }
}

#define DEFIRQ(n) DEFIDT(irq,32,n)
static void initIRQs() {
  idts[32] = taskGate(scheduleGate,0);
  DEFIRQ(1);
  DEFIRQ(2);
  DEFIRQ(3);
  DEFIRQ(4);
  DEFIRQ(5);
  DEFIRQ(6);
  DEFIRQ(7);
  DEFIRQ(8);
  DEFIRQ(9);
  DEFIRQ(10);
  DEFIRQ(11);
  DEFIRQ(12);
  DEFIRQ(13);
  DEFIRQ(14);
  DEFIRQ(15);
}
Feature _irqs_ = {
  .state = DISABLED,
  .label = "IRQs",
  .initialize = initIRQs
};
void handleIRQ(IDTParams* regs) {
  dword irqNum = regs->int_no;
  IDTHandler f = irqs[irqNum];
  
  if(f != 0) 
    f(regs);
  else
    printf("Caught unhandled IRQ %d\n",irqNum);
  
  if(irqNum >= 8) outportb(0xA0, 0x20);
  
  outportb(0x20, 0x20);
}

void sys_alloc(struct Task* t) {
  dword vpage = t->tss.ebx;
  void* newPage = allocatePage();

  mapPage(t->rr->univ,vpage,newPage);
}
