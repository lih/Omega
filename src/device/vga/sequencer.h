#ifndef INCLUDED_VGA_SEQUENCER
#define INCLUDED_VGA_SEQUENCER

#include <device/vga/common.h>

typedef struct {
  byte _:1;
  byte extmem:1;
  byte oedis:1;
  byte chain4:1;
} PACKED MemoryMode;
typedef struct {
  byte _98dm:1;
  byte _:1;
  byte slr:1;
  byte s4:1;
  byte sd:1;
} PACKED ClockingMode;

typedef struct {
  ClockingMode clockingMode;
  MemoryMode memoryMode;
  SetReset mapMask;
} PACKED SeqRegs;

void getSeqRegs(SeqRegs* regs);
void setSeqRegs(SeqRegs* regs);

void disableScreen();

#endif
