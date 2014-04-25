#include "constants.h"
#include "interrupt.h"
#include "framebuffer.h"
#include "pervasives.h"

IDTEntry idts[256];
typedef struct {
  word limit;
  IDTEntry* idts;
} __attribute__((__packed__)) IDTPtr;
IDTPtr idtp = {
  .limit = 256*sizeof(IDTEntry) - 1,
  .idts = &idts
};

void isr0(); void isr1(); void isr2(); void isr3(); void isr4(); void isr5(); void isr6();
void isr7(); void isr8(); void isr9(); void isr10(); void isr11(); void isr12(); void isr13();
void isr14(); void isr15(); void isr16(); void isr17(); void isr18(); void reservedISR();

void irq0(); void irq1(); void irq2(); void irq3(); void irq4(); void irq5();
void irq6(); void irq7(); void irq8(); void irq9(); void irq10(); void irq11();
void irq12(); void irq13(); void irq14(); void irq15(); void irq16();

void syscall0(); void syscall1(); void syscall2(); void syscall3(); void syscall4(); void syscall5(); void syscall6(); void syscall7(); void syscall8(); void syscall9(); void syscall10(); void syscall11(); void syscall12(); void syscall13(); void syscall14(); void syscall15(); void syscall16(); void syscall17(); void syscall18(); void syscall19(); void syscall20(); void syscall21(); void syscall22(); void syscall23(); void syscall24(); void syscall25(); void syscall26(); void syscall27(); void syscall28(); void syscall29(); void syscall30(); void syscall31(); void syscall32(); void syscall33(); void syscall34(); void syscall35(); void syscall36(); void syscall37(); void syscall38(); void syscall39(); void syscall40(); void syscall41(); void syscall42(); void syscall43(); void syscall44(); void syscall45(); void syscall46(); void syscall47(); void syscall48(); void syscall49(); void syscall50(); void syscall51(); void syscall52(); void syscall53(); void syscall54(); void syscall55(); void syscall56(); void syscall57(); void syscall58(); void syscall59(); void syscall60(); void syscall61(); void syscall62(); void syscall63(); void syscall64(); void syscall65(); void syscall66(); void syscall67(); void syscall68(); void syscall69(); void syscall70(); void syscall71(); void syscall72(); void syscall73(); void syscall74(); void syscall75(); void syscall76(); void syscall77(); void syscall78(); void syscall79(); void syscall80(); void syscall81(); void syscall82(); void syscall83(); void syscall84(); void syscall85(); void syscall86(); void syscall87(); void syscall88(); void syscall89(); void syscall90(); void syscall91(); void syscall92(); void syscall93(); void syscall94(); void syscall95(); void syscall96(); void syscall97(); void syscall98(); void syscall99(); void syscall100(); void syscall101(); void syscall102(); void syscall103(); void syscall104(); void syscall105(); void syscall106(); void syscall107(); void syscall108(); void syscall109(); void syscall110(); void syscall111(); void syscall112(); void syscall113(); void syscall114(); void syscall115(); void syscall116(); void syscall117(); void syscall118(); void syscall119(); void syscall120(); void syscall121(); void syscall122(); void syscall123(); void syscall124(); void syscall125(); void syscall126(); void syscall127(); void syscall128(); void syscall129(); void syscall130(); void syscall131(); void syscall132(); void syscall133(); void syscall134(); void syscall135(); void syscall136(); void syscall137(); void syscall138(); void syscall139(); void syscall140(); void syscall141(); void syscall142(); void syscall143(); void syscall144(); void syscall145(); void syscall146(); void syscall147(); void syscall148(); void syscall149(); void syscall150(); void syscall151(); void syscall152(); void syscall153(); void syscall154(); void syscall155(); void syscall156(); void syscall157(); void syscall158(); void syscall159(); void syscall160(); void syscall161(); void syscall162(); void syscall163(); void syscall164(); void syscall165(); void syscall166(); void syscall167(); void syscall168(); void syscall169(); void syscall170(); void syscall171(); void syscall172(); void syscall173(); void syscall174(); void syscall175(); void syscall176(); void syscall177(); void syscall178(); void syscall179(); void syscall180(); void syscall181(); void syscall182(); void syscall183(); void syscall184(); void syscall185(); void syscall186(); void syscall187(); void syscall188(); void syscall189(); void syscall190(); void syscall191(); void syscall192(); void syscall193(); void syscall194(); void syscall195(); void syscall196(); void syscall197(); void syscall198(); void syscall199(); void syscall200(); void syscall201(); void syscall202(); void syscall203(); void syscall204(); void syscall205(); void syscall206(); void syscall207();

char* descriptions[32]; 
IDTHandler irqs[16] = { 0 };
IDTHandler syscalls[207] = { 0 };

void initExceptions();
void initIRQs();
void initSyscalls();

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

void initInterrupts() {
  initExceptions();
  initIRQs();
  initSyscalls();
 
  idtp.limit = 256*sizeof(IDTEntry) - 1;
  idtp.idts = &idts;
  loadInterrupts(&idtp);

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

void handleISR(IDTParams* regs) {
  dword ex_num = regs->int_no;

  if(ex_num < 32) {
    printf("Caught Exception %d (%s) with error code %d\n",ex_num,descriptions[ex_num],regs->err_code);
    printf("eax=%x, esp=%x, eip=%x",regs->eax,regs->useresp,regs->eip);
    while(1) {
      nop();
    }
  }
}
void handleIRQ(IDTParams* regs) {
  dword irqNum = regs->int_no;
  IDTHandler f = irqs[irqNum];
  
  if(f != 0) 
    f(regs);
  else
    printf("Caught unhandled IRQ %d\n",irqNum);
  
  if(irqNum >= 8) {
    outportb(0xA0, 0x20);
  }
  outportb(0x20, 0x20);
}
void handleSyscall(IDTParams* regs) {
  dword syscallNum = regs->int_no;
  IDTHandler f = irqs[syscallNum];
  
  if(f != 0) 
    f(regs);
  else
    printf("Caught unhandled syscall %d\n",syscallNum);
}

void initExceptions() {
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
}
void initIRQs() {
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
}
void initSyscalls() {
  idts[48] = idtEntry((dword)syscall0,CODE_SEGMENT,0,1);
  idts[49] = idtEntry((dword)syscall1,CODE_SEGMENT,0,1);
  idts[50] = idtEntry((dword)syscall2,CODE_SEGMENT,0,1);
  idts[51] = idtEntry((dword)syscall3,CODE_SEGMENT,0,1);
  idts[52] = idtEntry((dword)syscall4,CODE_SEGMENT,0,1);
  idts[53] = idtEntry((dword)syscall5,CODE_SEGMENT,0,1);
  idts[54] = idtEntry((dword)syscall6,CODE_SEGMENT,0,1);
  idts[55] = idtEntry((dword)syscall7,CODE_SEGMENT,0,1);
  idts[56] = idtEntry((dword)syscall8,CODE_SEGMENT,0,1);
  idts[57] = idtEntry((dword)syscall9,CODE_SEGMENT,0,1);
  idts[58] = idtEntry((dword)syscall10,CODE_SEGMENT,0,1);
  idts[59] = idtEntry((dword)syscall11,CODE_SEGMENT,0,1);
  idts[60] = idtEntry((dword)syscall12,CODE_SEGMENT,0,1);
  idts[61] = idtEntry((dword)syscall13,CODE_SEGMENT,0,1);
  idts[62] = idtEntry((dword)syscall14,CODE_SEGMENT,0,1);
  idts[63] = idtEntry((dword)syscall15,CODE_SEGMENT,0,1);
  idts[64] = idtEntry((dword)syscall16,CODE_SEGMENT,0,1);
  idts[65] = idtEntry((dword)syscall17,CODE_SEGMENT,0,1);
  idts[66] = idtEntry((dword)syscall18,CODE_SEGMENT,0,1);
  idts[67] = idtEntry((dword)syscall19,CODE_SEGMENT,0,1);
  idts[68] = idtEntry((dword)syscall20,CODE_SEGMENT,0,1);
  idts[69] = idtEntry((dword)syscall21,CODE_SEGMENT,0,1);
  idts[70] = idtEntry((dword)syscall22,CODE_SEGMENT,0,1);
  idts[71] = idtEntry((dword)syscall23,CODE_SEGMENT,0,1);
  idts[72] = idtEntry((dword)syscall24,CODE_SEGMENT,0,1);
  idts[73] = idtEntry((dword)syscall25,CODE_SEGMENT,0,1);
  idts[74] = idtEntry((dword)syscall26,CODE_SEGMENT,0,1);
  idts[75] = idtEntry((dword)syscall27,CODE_SEGMENT,0,1);
  idts[76] = idtEntry((dword)syscall28,CODE_SEGMENT,0,1);
  idts[77] = idtEntry((dword)syscall29,CODE_SEGMENT,0,1);
  idts[78] = idtEntry((dword)syscall30,CODE_SEGMENT,0,1);
  idts[79] = idtEntry((dword)syscall31,CODE_SEGMENT,0,1);
  idts[80] = idtEntry((dword)syscall32,CODE_SEGMENT,0,1);
  idts[81] = idtEntry((dword)syscall33,CODE_SEGMENT,0,1);
  idts[82] = idtEntry((dword)syscall34,CODE_SEGMENT,0,1);
  idts[83] = idtEntry((dword)syscall35,CODE_SEGMENT,0,1);
  idts[84] = idtEntry((dword)syscall36,CODE_SEGMENT,0,1);
  idts[85] = idtEntry((dword)syscall37,CODE_SEGMENT,0,1);
  idts[86] = idtEntry((dword)syscall38,CODE_SEGMENT,0,1);
  idts[87] = idtEntry((dword)syscall39,CODE_SEGMENT,0,1);
  idts[88] = idtEntry((dword)syscall40,CODE_SEGMENT,0,1);
  idts[89] = idtEntry((dword)syscall41,CODE_SEGMENT,0,1);
  idts[90] = idtEntry((dword)syscall42,CODE_SEGMENT,0,1);
  idts[91] = idtEntry((dword)syscall43,CODE_SEGMENT,0,1);
  idts[92] = idtEntry((dword)syscall44,CODE_SEGMENT,0,1);
  idts[93] = idtEntry((dword)syscall45,CODE_SEGMENT,0,1);
  idts[94] = idtEntry((dword)syscall46,CODE_SEGMENT,0,1);
  idts[95] = idtEntry((dword)syscall47,CODE_SEGMENT,0,1);
  idts[96] = idtEntry((dword)syscall48,CODE_SEGMENT,0,1);
  idts[97] = idtEntry((dword)syscall49,CODE_SEGMENT,0,1);
  idts[98] = idtEntry((dword)syscall50,CODE_SEGMENT,0,1);
  idts[99] = idtEntry((dword)syscall51,CODE_SEGMENT,0,1);
  idts[100] = idtEntry((dword)syscall52,CODE_SEGMENT,0,1);
  idts[101] = idtEntry((dword)syscall53,CODE_SEGMENT,0,1);
  idts[102] = idtEntry((dword)syscall54,CODE_SEGMENT,0,1);
  idts[103] = idtEntry((dword)syscall55,CODE_SEGMENT,0,1);
  idts[104] = idtEntry((dword)syscall56,CODE_SEGMENT,0,1);
  idts[105] = idtEntry((dword)syscall57,CODE_SEGMENT,0,1);
  idts[106] = idtEntry((dword)syscall58,CODE_SEGMENT,0,1);
  idts[107] = idtEntry((dword)syscall59,CODE_SEGMENT,0,1);
  idts[108] = idtEntry((dword)syscall60,CODE_SEGMENT,0,1);
  idts[109] = idtEntry((dword)syscall61,CODE_SEGMENT,0,1);
  idts[110] = idtEntry((dword)syscall62,CODE_SEGMENT,0,1);
  idts[111] = idtEntry((dword)syscall63,CODE_SEGMENT,0,1);
  idts[112] = idtEntry((dword)syscall64,CODE_SEGMENT,0,1);
  idts[113] = idtEntry((dword)syscall65,CODE_SEGMENT,0,1);
  idts[114] = idtEntry((dword)syscall66,CODE_SEGMENT,0,1);
  idts[115] = idtEntry((dword)syscall67,CODE_SEGMENT,0,1);
  idts[116] = idtEntry((dword)syscall68,CODE_SEGMENT,0,1);
  idts[117] = idtEntry((dword)syscall69,CODE_SEGMENT,0,1);
  idts[118] = idtEntry((dword)syscall70,CODE_SEGMENT,0,1);
  idts[119] = idtEntry((dword)syscall71,CODE_SEGMENT,0,1);
  idts[120] = idtEntry((dword)syscall72,CODE_SEGMENT,0,1);
  idts[121] = idtEntry((dword)syscall73,CODE_SEGMENT,0,1);
  idts[122] = idtEntry((dword)syscall74,CODE_SEGMENT,0,1);
  idts[123] = idtEntry((dword)syscall75,CODE_SEGMENT,0,1);
  idts[124] = idtEntry((dword)syscall76,CODE_SEGMENT,0,1);
  idts[125] = idtEntry((dword)syscall77,CODE_SEGMENT,0,1);
  idts[126] = idtEntry((dword)syscall78,CODE_SEGMENT,0,1);
  idts[127] = idtEntry((dword)syscall79,CODE_SEGMENT,0,1);
  idts[128] = idtEntry((dword)syscall80,CODE_SEGMENT,0,1);
  idts[129] = idtEntry((dword)syscall81,CODE_SEGMENT,0,1);
  idts[130] = idtEntry((dword)syscall82,CODE_SEGMENT,0,1);
  idts[131] = idtEntry((dword)syscall83,CODE_SEGMENT,0,1);
  idts[132] = idtEntry((dword)syscall84,CODE_SEGMENT,0,1);
  idts[133] = idtEntry((dword)syscall85,CODE_SEGMENT,0,1);
  idts[134] = idtEntry((dword)syscall86,CODE_SEGMENT,0,1);
  idts[135] = idtEntry((dword)syscall87,CODE_SEGMENT,0,1);
  idts[136] = idtEntry((dword)syscall88,CODE_SEGMENT,0,1);
  idts[137] = idtEntry((dword)syscall89,CODE_SEGMENT,0,1);
  idts[138] = idtEntry((dword)syscall90,CODE_SEGMENT,0,1);
  idts[139] = idtEntry((dword)syscall91,CODE_SEGMENT,0,1);
  idts[140] = idtEntry((dword)syscall92,CODE_SEGMENT,0,1);
  idts[141] = idtEntry((dword)syscall93,CODE_SEGMENT,0,1);
  idts[142] = idtEntry((dword)syscall94,CODE_SEGMENT,0,1);
  idts[143] = idtEntry((dword)syscall95,CODE_SEGMENT,0,1);
  idts[144] = idtEntry((dword)syscall96,CODE_SEGMENT,0,1);
  idts[145] = idtEntry((dword)syscall97,CODE_SEGMENT,0,1);
  idts[146] = idtEntry((dword)syscall98,CODE_SEGMENT,0,1);
  idts[147] = idtEntry((dword)syscall99,CODE_SEGMENT,0,1);
  idts[148] = idtEntry((dword)syscall100,CODE_SEGMENT,0,1);
  idts[149] = idtEntry((dword)syscall101,CODE_SEGMENT,0,1);
  idts[150] = idtEntry((dword)syscall102,CODE_SEGMENT,0,1);
  idts[151] = idtEntry((dword)syscall103,CODE_SEGMENT,0,1);
  idts[152] = idtEntry((dword)syscall104,CODE_SEGMENT,0,1);
  idts[153] = idtEntry((dword)syscall105,CODE_SEGMENT,0,1);
  idts[154] = idtEntry((dword)syscall106,CODE_SEGMENT,0,1);
  idts[155] = idtEntry((dword)syscall107,CODE_SEGMENT,0,1);
  idts[156] = idtEntry((dword)syscall108,CODE_SEGMENT,0,1);
  idts[157] = idtEntry((dword)syscall109,CODE_SEGMENT,0,1);
  idts[158] = idtEntry((dword)syscall110,CODE_SEGMENT,0,1);
  idts[159] = idtEntry((dword)syscall111,CODE_SEGMENT,0,1);
  idts[160] = idtEntry((dword)syscall112,CODE_SEGMENT,0,1);
  idts[161] = idtEntry((dword)syscall113,CODE_SEGMENT,0,1);
  idts[162] = idtEntry((dword)syscall114,CODE_SEGMENT,0,1);
  idts[163] = idtEntry((dword)syscall115,CODE_SEGMENT,0,1);
  idts[164] = idtEntry((dword)syscall116,CODE_SEGMENT,0,1);
  idts[165] = idtEntry((dword)syscall117,CODE_SEGMENT,0,1);
  idts[166] = idtEntry((dword)syscall118,CODE_SEGMENT,0,1);
  idts[167] = idtEntry((dword)syscall119,CODE_SEGMENT,0,1);
  idts[168] = idtEntry((dword)syscall120,CODE_SEGMENT,0,1);
  idts[169] = idtEntry((dword)syscall121,CODE_SEGMENT,0,1);
  idts[170] = idtEntry((dword)syscall122,CODE_SEGMENT,0,1);
  idts[171] = idtEntry((dword)syscall123,CODE_SEGMENT,0,1);
  idts[172] = idtEntry((dword)syscall124,CODE_SEGMENT,0,1);
  idts[173] = idtEntry((dword)syscall125,CODE_SEGMENT,0,1);
  idts[174] = idtEntry((dword)syscall126,CODE_SEGMENT,0,1);
  idts[175] = idtEntry((dword)syscall127,CODE_SEGMENT,0,1);
  idts[176] = idtEntry((dword)syscall128,CODE_SEGMENT,0,1);
  idts[177] = idtEntry((dword)syscall129,CODE_SEGMENT,0,1);
  idts[178] = idtEntry((dword)syscall130,CODE_SEGMENT,0,1);
  idts[179] = idtEntry((dword)syscall131,CODE_SEGMENT,0,1);
  idts[180] = idtEntry((dword)syscall132,CODE_SEGMENT,0,1);
  idts[181] = idtEntry((dword)syscall133,CODE_SEGMENT,0,1);
  idts[182] = idtEntry((dword)syscall134,CODE_SEGMENT,0,1);
  idts[183] = idtEntry((dword)syscall135,CODE_SEGMENT,0,1);
  idts[184] = idtEntry((dword)syscall136,CODE_SEGMENT,0,1);
  idts[185] = idtEntry((dword)syscall137,CODE_SEGMENT,0,1);
  idts[186] = idtEntry((dword)syscall138,CODE_SEGMENT,0,1);
  idts[187] = idtEntry((dword)syscall139,CODE_SEGMENT,0,1);
  idts[188] = idtEntry((dword)syscall140,CODE_SEGMENT,0,1);
  idts[189] = idtEntry((dword)syscall141,CODE_SEGMENT,0,1);
  idts[190] = idtEntry((dword)syscall142,CODE_SEGMENT,0,1);
  idts[191] = idtEntry((dword)syscall143,CODE_SEGMENT,0,1);
  idts[192] = idtEntry((dword)syscall144,CODE_SEGMENT,0,1);
  idts[193] = idtEntry((dword)syscall145,CODE_SEGMENT,0,1);
  idts[194] = idtEntry((dword)syscall146,CODE_SEGMENT,0,1);
  idts[195] = idtEntry((dword)syscall147,CODE_SEGMENT,0,1);
  idts[196] = idtEntry((dword)syscall148,CODE_SEGMENT,0,1);
  idts[197] = idtEntry((dword)syscall149,CODE_SEGMENT,0,1);
  idts[198] = idtEntry((dword)syscall150,CODE_SEGMENT,0,1);
  idts[199] = idtEntry((dword)syscall151,CODE_SEGMENT,0,1);
  idts[200] = idtEntry((dword)syscall152,CODE_SEGMENT,0,1);
  idts[201] = idtEntry((dword)syscall153,CODE_SEGMENT,0,1);
  idts[202] = idtEntry((dword)syscall154,CODE_SEGMENT,0,1);
  idts[203] = idtEntry((dword)syscall155,CODE_SEGMENT,0,1);
  idts[204] = idtEntry((dword)syscall156,CODE_SEGMENT,0,1);
  idts[205] = idtEntry((dword)syscall157,CODE_SEGMENT,0,1);
  idts[206] = idtEntry((dword)syscall158,CODE_SEGMENT,0,1);
  idts[207] = idtEntry((dword)syscall159,CODE_SEGMENT,0,1);
  idts[208] = idtEntry((dword)syscall160,CODE_SEGMENT,0,1);
  idts[209] = idtEntry((dword)syscall161,CODE_SEGMENT,0,1);
  idts[210] = idtEntry((dword)syscall162,CODE_SEGMENT,0,1);
  idts[211] = idtEntry((dword)syscall163,CODE_SEGMENT,0,1);
  idts[212] = idtEntry((dword)syscall164,CODE_SEGMENT,0,1);
  idts[213] = idtEntry((dword)syscall165,CODE_SEGMENT,0,1);
  idts[214] = idtEntry((dword)syscall166,CODE_SEGMENT,0,1);
  idts[215] = idtEntry((dword)syscall167,CODE_SEGMENT,0,1);
  idts[216] = idtEntry((dword)syscall168,CODE_SEGMENT,0,1);
  idts[217] = idtEntry((dword)syscall169,CODE_SEGMENT,0,1);
  idts[218] = idtEntry((dword)syscall170,CODE_SEGMENT,0,1);
  idts[219] = idtEntry((dword)syscall171,CODE_SEGMENT,0,1);
  idts[220] = idtEntry((dword)syscall172,CODE_SEGMENT,0,1);
  idts[221] = idtEntry((dword)syscall173,CODE_SEGMENT,0,1);
  idts[222] = idtEntry((dword)syscall174,CODE_SEGMENT,0,1);
  idts[223] = idtEntry((dword)syscall175,CODE_SEGMENT,0,1);
  idts[224] = idtEntry((dword)syscall176,CODE_SEGMENT,0,1);
  idts[225] = idtEntry((dword)syscall177,CODE_SEGMENT,0,1);
  idts[226] = idtEntry((dword)syscall178,CODE_SEGMENT,0,1);
  idts[227] = idtEntry((dword)syscall179,CODE_SEGMENT,0,1);
  idts[228] = idtEntry((dword)syscall180,CODE_SEGMENT,0,1);
  idts[229] = idtEntry((dword)syscall181,CODE_SEGMENT,0,1);
  idts[230] = idtEntry((dword)syscall182,CODE_SEGMENT,0,1);
  idts[231] = idtEntry((dword)syscall183,CODE_SEGMENT,0,1);
  idts[232] = idtEntry((dword)syscall184,CODE_SEGMENT,0,1);
  idts[233] = idtEntry((dword)syscall185,CODE_SEGMENT,0,1);
  idts[234] = idtEntry((dword)syscall186,CODE_SEGMENT,0,1);
  idts[235] = idtEntry((dword)syscall187,CODE_SEGMENT,0,1);
  idts[236] = idtEntry((dword)syscall188,CODE_SEGMENT,0,1);
  idts[237] = idtEntry((dword)syscall189,CODE_SEGMENT,0,1);
  idts[238] = idtEntry((dword)syscall190,CODE_SEGMENT,0,1);
  idts[239] = idtEntry((dword)syscall191,CODE_SEGMENT,0,1);
  idts[240] = idtEntry((dword)syscall192,CODE_SEGMENT,0,1);
  idts[241] = idtEntry((dword)syscall193,CODE_SEGMENT,0,1);
  idts[242] = idtEntry((dword)syscall194,CODE_SEGMENT,0,1);
  idts[243] = idtEntry((dword)syscall195,CODE_SEGMENT,0,1);
  idts[244] = idtEntry((dword)syscall196,CODE_SEGMENT,0,1);
  idts[245] = idtEntry((dword)syscall197,CODE_SEGMENT,0,1);
  idts[246] = idtEntry((dword)syscall198,CODE_SEGMENT,0,1);
  idts[247] = idtEntry((dword)syscall199,CODE_SEGMENT,0,1);
  idts[248] = idtEntry((dword)syscall200,CODE_SEGMENT,0,1);
  idts[249] = idtEntry((dword)syscall201,CODE_SEGMENT,0,1);
  idts[250] = idtEntry((dword)syscall202,CODE_SEGMENT,0,1);
  idts[251] = idtEntry((dword)syscall203,CODE_SEGMENT,0,1);
  idts[252] = idtEntry((dword)syscall204,CODE_SEGMENT,0,1);
  idts[253] = idtEntry((dword)syscall205,CODE_SEGMENT,0,1);
  idts[254] = idtEntry((dword)syscall206,CODE_SEGMENT,0,1);
  idts[255] = idtEntry((dword)syscall207,CODE_SEGMENT,0,1);
}
