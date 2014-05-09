#include <device/vga/attributes.h>

VGAReg attributeMode = REG(ACR,0x30);
VGAReg overscanIndex = REG(ACR,0x31);
VGAReg colorPlaneEnable = REG(ACR,0x32);

void setACRRegs(ACRRegs* regs) {
  inportb(INPUT_STATUS_1);
  byte oldAddr = inportb(ACR_ADDR);
  SETREG(regs,attributeMode);
  SETREG(regs,overscanIndex);
  SETREG(regs,colorPlaneEnable);
  outportb(ACR_ADDR,oldAddr);
}
void getACRRegs(ACRRegs* regs) {
  inportb(INPUT_STATUS_1);
  byte oldAddr = inportb(ACR_ADDR);
  GETREG(regs,attributeMode);
  GETREG(regs,overscanIndex);
  GETREG(regs,colorPlaneEnable);
  outportb(ACR_ADDR,oldAddr);
}
