#include <device/vga/gc.h>

VGAReg setReset		= REG(VGA,0);
VGAReg enableSetReset	= REG(VGA,1);
VGAReg colorCompare	= REG(VGA,2);
VGAReg dataRotate	= REG(VGA,3);
VGAReg readMap		= REG(VGA,4);
VGAReg graphicsMode	= REG(VGA,5);
VGAReg misc		= REG(VGA,6);
VGAReg dontCare		= REG(VGA,7);
VGAReg bitmask		= REG(VGA,8);

void setGCRegs(GCRegs* regs) {
  SETREG(regs,setReset);
  SETREG(regs,enableSetReset);
  SETREG(regs,colorCompare);
  SETREG(regs,readMap);
  SETREG(regs,graphicsMode);
  SETREG(regs,misc);
  SETREG(regs,dontCare);
  SETREG(regs,bitmask);
}
void getGCRegs(GCRegs* regs) {
  GETREG(regs,setReset);
  GETREG(regs,enableSetReset);
  GETREG(regs,colorCompare);
  GETREG(regs,readMap);
  GETREG(regs,graphicsMode);
  GETREG(regs,misc);
  GETREG(regs,dontCare);
  GETREG(regs,bitmask);
}
