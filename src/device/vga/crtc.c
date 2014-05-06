#include <device/vga/crtc.h>

VGAReg hTotal		= REG(CRT,0);
VGAReg endHDisplay	= REG(CRT,1);
VGAReg startHBlanking	= REG(CRT,2);
VGAReg startHRetrace	= REG(CRT,4);
VGAReg endHRetrace	= REG(CRT,5);
VGAReg endHBlanking	= REG(CRT,3);

VGAReg vTotal		= REG(CRT,6);
VGAReg endVDisplay	= REG(CRT,0x12);
VGAReg startVBlanking	= REG(CRT,0x15);
VGAReg startVRetrace	= REG(CRT,0x10);
VGAReg endVRetrace	= REG(CRT,0x11);
VGAReg endVBlanking	= REG(CRT,0x16);

VGAReg offset			= REG(CRT,0x13);
VGAReg underlineLocation	= REG(CRT,0x14);
VGAReg presetRowScan		= REG(CRT,8);
VGAReg overflow			= REG(CRT,7);
VGAReg maxScanLine		= REG(CRT,9);
VGAReg crtcMode			= REG(CRT,0x17);

void getCRTCRegs(CRTCRegs* regs) {
  GETREG(regs,hTotal);
  GETREG(regs,endHDisplay);
  GETREG(regs,startHBlanking);
  GETREG(regs,startHRetrace);
  GETREG(regs,endHRetrace);
  GETREG(regs,endHBlanking);

  GETREG(regs,vTotal);
  GETREG(regs,endVDisplay);
  GETREG(regs,startVBlanking);
  GETREG(regs,startVRetrace);
  GETREG(regs,endVRetrace);
  GETREG(regs,endVBlanking);

  GETREG(regs,offset);
  GETREG(regs,underlineLocation);
  GETREG(regs,presetRowScan);
  GETREG(regs,overflow);
  GETREG(regs,maxScanLine);
  GETREG(regs,crtcMode);
}
void setCRTCRegs(CRTCRegs* regs) {
  /* Unlock CRT registers */
  MODREG(endHBlanking,EndHBlanking,REG.evra = 1);
  MODREG(endVRetrace,EndVRetrace,REG.protect = 0);

  regs->endHBlanking.evra = 1;
  regs->endVRetrace.protect = 0;
  
  SETREG(regs,hTotal);
  SETREG(regs,endHDisplay);
  SETREG(regs,startHBlanking);
  SETREG(regs,startHRetrace);
  SETREG(regs,endHRetrace);
  SETREG(regs,endHBlanking);
  SETREG(regs,vTotal);
  SETREG(regs,endVDisplay);
  SETREG(regs,startVBlanking);
  SETREG(regs,startVRetrace);
  SETREG(regs,endVRetrace);
  SETREG(regs,endVBlanking);
  
  SETREG(regs,offset);
  SETREG(regs,underlineLocation);
  SETREG(regs,presetRowScan);
  SETREG(regs,overflow);
  SETREG(regs,maxScanLine);
  SETREG(regs,crtcMode);
}
