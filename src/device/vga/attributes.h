#ifndef INCLUDED_ATTRIBUTES
#define INCLUDED_ATTRIBUTES

#include <device/vga/common.h>

typedef struct {
  byte atge:1;
  byte mono:1;
  byte lge:1;
  byte blink:1;
  byte _:1;
  byte ppm:1;
  byte _8bit:1;
  byte p54s:1;
} PACKED AttributeMode;

typedef struct {
  SetReset colorPlaneEnable;
  AttributeMode attributeMode;
  byte overscanIndex;
} PACKED ACRRegs;

void setACRRegs(ACRRegs* regs);
void getACRRegs(ACRRegs* regs);

#endif
