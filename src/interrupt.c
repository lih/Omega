#include "constants.h"
#include "interrupt.h"
#include "framebuffer.h"
#include "pervasives.h"

IDTEntry idtEntry(dword base,word sel,byte dpl,byte present) {
  IDTEntry ret = {
    .base_lo = base & 0xffff,
    .sel = sel,
    ._zero = 0,
    .flags = {
      ._fourteen = 14,
      .dpl = dpl,
      .present = present
    },
    .base_hi = base >> 16
  };

  return ret;
}

IDTEntry idts[256];
typedef struct {
  word limit;
  IDTEntry* idts;
} __attribute__((__packed__)) IDTPtr;
IDTPtr idtp = {
  .limit = 256*sizeof(IDTEntry) - 1,
  .idts = &idts
};

void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void reservedISR();
void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();
void otherIRQ();

char* descriptions[32]; 

void remapIRQs(void) {
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
}

void init_interrupts() {
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

  idts[0] = idtEntry((dword)isr0,CODE_SEGMENT,0,1);
  idts[1] = idtEntry((dword)isr1,CODE_SEGMENT,0,1);
  idts[2] = idtEntry((dword)isr2,CODE_SEGMENT,0,1);
  idts[3] = idtEntry((dword)isr3,CODE_SEGMENT,0,1);
  idts[4] = idtEntry((dword)isr4,CODE_SEGMENT,0,1);
  idts[5] = idtEntry((dword)isr5,CODE_SEGMENT,0,1);
  idts[6] = idtEntry((dword)isr6,CODE_SEGMENT,0,1);
  idts[7] = idtEntry((dword)isr7,CODE_SEGMENT,0,1);
  idts[8] = idtEntry((dword)isr8,CODE_SEGMENT,0,1);
  idts[9] = idtEntry((dword)isr9,CODE_SEGMENT,0,1);
  idts[10] = idtEntry((dword)isr10,CODE_SEGMENT,0,1);
  idts[11] = idtEntry((dword)isr11,CODE_SEGMENT,0,1);
  idts[12] = idtEntry((dword)isr12,CODE_SEGMENT,0,1);
  idts[13] = idtEntry((dword)isr13,CODE_SEGMENT,0,1);
  idts[14] = idtEntry((dword)isr14,CODE_SEGMENT,0,1);
  idts[15] = idtEntry((dword)isr15,CODE_SEGMENT,0,1);
  idts[16] = idtEntry((dword)isr16,CODE_SEGMENT,0,1);
  idts[17] = idtEntry((dword)isr17,CODE_SEGMENT,0,1);
  idts[18] = idtEntry((dword)isr18,CODE_SEGMENT,0,1);
  
  int i;
  for(i=19;i<32;i++) {
    idts[i] = idtEntry((dword)reservedISR,CODE_SEGMENT,0,1);
    descriptions[i]="Reserved";
  }
  
  idts[32] = idtEntry((dword)irq0,CODE_SEGMENT,0,1);
  idts[33] = idtEntry((dword)irq1,CODE_SEGMENT,0,1);
  idts[34] = idtEntry((dword)irq2,CODE_SEGMENT,0,1);
  idts[35] = idtEntry((dword)irq3,CODE_SEGMENT,0,1);
  idts[36] = idtEntry((dword)irq4,CODE_SEGMENT,0,1);
  idts[37] = idtEntry((dword)irq5,CODE_SEGMENT,0,1);
  idts[38] = idtEntry((dword)irq6,CODE_SEGMENT,0,1);
  idts[39] = idtEntry((dword)irq7,CODE_SEGMENT,0,1);
  idts[40] = idtEntry((dword)irq8,CODE_SEGMENT,0,1);
  idts[41] = idtEntry((dword)irq9,CODE_SEGMENT,0,1);
  idts[42] = idtEntry((dword)irq10,CODE_SEGMENT,0,1);
  idts[43] = idtEntry((dword)irq11,CODE_SEGMENT,0,1);
  idts[44] = idtEntry((dword)irq12,CODE_SEGMENT,0,1);
  idts[45] = idtEntry((dword)irq13,CODE_SEGMENT,0,1);
  idts[46] = idtEntry((dword)irq14,CODE_SEGMENT,0,1);
  idts[47] = idtEntry((dword)irq15,CODE_SEGMENT,0,1);
  
  for(i=48;i<256;i++)
    idts[i] = idtEntry((dword)otherIRQ,CODE_SEGMENT,0,1);
 
  idtp.limit = 48*sizeof(IDTEntry) - 1;
  idtp.idts = &idts;

  remapIRQs();
  loadInterrupts();
  enableInterrupts();
}

IDTHandler irqs[16] = { 0 };

void handleISR(IDTParams* regs) {
  dword ex_num = regs->int_no;

  if(ex_num < 32) {
    printf("Caught Exception %d (%s) with error code %d\n",ex_num,descriptions[ex_num],regs->err_code);
    printf("eax=%d, esp=%d, eip=%d",regs->eax,regs->useresp,regs->eip);
    while(1) {
      nop();
    }
  }
  /* else { */
  /*   printf("Unhandled exception %d\n",ex_num); */
  /* } */
}
void handleIRQ(IDTParams* regs) {
  dword irqNum = regs->int_no;
  
  if(irqNum < 16) {
    IDTHandler f = irqs[irqNum];
    if(f != 0) 
      f(regs);
  }
  /* else { */
  /*   printf("Unhandled interrupt.\n"); */
  /* } */

  if(irqNum >= 8) {
    outportb(0xA0, 0x20);
  }
  outportb(0x20, 0x20);
}
