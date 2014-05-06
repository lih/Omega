#include <device/vga/attributes.h>

VGAReg attributeMode = REG(ACR,0x30);
VGAReg overscanIndex = REG(ACR,0x31);
VGAReg colorPlaneEnable = REG(ACR,0x32);

void setACRRegs(ACRRegs* regs) {
  SETREG(regs,attributeMode);
  SETREG(regs,overscanIndex);
  SETREG(regs,colorPlaneEnable);
}
void getACRRegs(ACRRegs* regs) {
  SETREG(regs,attributeMode);
  SETREG(regs,overscanIndex);
  SETREG(regs,colorPlaneEnable);
}
