#include <constants.h>
#include <core/feature.h>
#include <core/schedule.h>
#include <cpu/interrupt.h>
#include <cpu/pervasives.h>
#include <cpu/syscall.h>
#include <device/framebuffer.h>
#include <device/keyboard.h>

TablePtr idt = {
  .limit = 49*sizeof(Descriptor) - 1,
  .base = idts
};

char* descriptions[32] = {
  "Division by Zero",
  "Debug",
  "Non Maskable Interrupt",
  "Breakpoint",
  "Overflow",
  "Out of Bounds",
  "Invalid Opcode",
  "No Coprocessor",
  "Double Fault",
  "Coprocessor Segment Overrun",
  "Bad TSS",
  "Segment Not Present",
  "Stack Fault",
  "General Protection Fault",
  "Page Fault",
  "Unknown Interrupt",
  "Coprocessor Fault",
  "Alignment Check",
  "Machine Check",
  
  "Reserved","Reserved","Reserved","Reserved"
  ,"Reserved","Reserved","Reserved","Reserved"
  ,"Reserved","Reserved","Reserved","Reserved"
  ,"Reserved"
};
IDTHandler irqs[16] = { 0 };

static void initInterrupts() {
  require(&_exceptions_);
  require(&_irqs_);
  require(&_syscalls_);
   
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

  flushGDT();
  loadInterrupts();
}
Feature _interrupts_ = {
  .state = DISABLED,
  .label = "interrupts",
  .initialize = initInterrupts
};

#define DEFIDT(base,off,n) \
  idts[off+n] = interruptGate(CODE_SEGMENT,base##n,0)

void unhandledException();
static void initExceptions() {
  require(&_schedule_);

  TSS* t = (void*)EXC_STACK - sizeof(TSS);
  *t = tss(kernelSpace.pageDir,unhandledException,t);
  Selector excGate = addDesc(&gdt,tssDesc(t,0));
  
  int i;
  for(i=0;i<32;i++)
    idts[i] = taskGate(excGate,0);
}
Feature _exceptions_ = {
  .state = DISABLED,
  .label = "exceptions",
  .initialize = &initExceptions
};

void handleException() {
  TSS* tss = TSS_AT(getTaskRegister());
  while(1) {
    TSS* child = TSS_AT(tss->previousTask);
  
    printf("Caught exception in TSS %x\n"
	   "eax=%x ebx=%x ecx=%x edx=%x\n"
	   "esp=%x ss=%x eip=%x cs=%x\n",
	   child,child->eax,child->ebx,child->ecx,child->edx,
	   child->esp,child->ss,child->eip,child->cs);

    while(1);
    asm __volatile__ ( "iret" );
  }
}

void unhandledIRQ() {
  TSS* ss = TSS_AT(getTaskRegister());
  while(1) {
    (void)ss;
    asm __volatile__ ( "iret" );
  }
}

#define DEFIRQ(n) idts[n+32] = taskGate(irqGate,0);
static void initIRQs() {
  require(&_schedule_);
  require(&_keyboard_);

  TSS* irqTSS = (void*)IRQ_STACK - sizeof(TSS);
  *irqTSS = tss(kernelSpace.pageDir,unhandledIRQ,irqTSS);
  Selector irqGate = addDesc(&gdt,tssDesc(irqTSS,0));

  idts[32] = taskGate(scheduleGate,0);
  idts[33] = taskGate(keyboardGate,0);
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
