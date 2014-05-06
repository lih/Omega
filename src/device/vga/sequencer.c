#include <device/vga/sequencer.h>

VGAReg clockingMode = REG(SEQ,1);
VGAReg memoryMode = REG(SEQ,4);
VGAReg mapMask = REG(SEQ,2);

void getSeqRegs(SeqRegs* regs) {
  GETREG(regs,clockingMode);
  GETREG(regs,memoryMode);
  GETREG(regs,mapMask);
}
void setSeqRegs(SeqRegs* regs) {
  SETREG(regs,clockingMode);
  SETREG(regs,memoryMode);
  SETREG(regs,mapMask);
}
