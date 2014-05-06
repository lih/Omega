#ifndef INCLUDED_CRTC
#define INCLUDED_CRTC

#include <device/vga/common.h>

#define CRT_ADDR 0x3d4
#define CRT_DATA 0x3d5

typedef struct {
  byte endHBlanking:5;
  byte displaySkew:2;
  byte evra:1;
} PACKED EndHBlanking;
typedef struct {
  byte endHRetrace:5;
  byte hSkew:2;
  byte ehb5:1;
} PACKED EndHRetrace;
typedef struct { 
  byte endVRetrace:4;
  byte _:2;
  byte bandwidth:1;
  byte protect:1;
} PACKED EndVRetrace;
typedef struct {
  byte underlineLocation:5;
  byte div4:1;
  byte dw:1;
} PACKED UnderlineLocation;
typedef struct {
  byte maxScanLine:5;
  byte svb9:1;
  byte lc9:1;
  byte sd:1;
} PACKED MaxScanLine;
typedef struct {
  byte presetRowScan:5;
  byte panning:2;
} PACKED PresetRowScan;
typedef struct {
  byte vt8:1;
  byte vde8:1;
  byte vrs8:1;
  byte svb8:1;
  byte lc8:1;
  byte vt9:1;
  byte vde9:1;
  byte vrs9:1;
} PACKED Overflow;
typedef struct { 
  byte map13:1;
  byte map14:1;
  byte sldiv:1;
  byte div2:1;
  byte _:1;
  byte aw:1;
  byte wordByte:1;
  byte se:1;
} PACKED CRTCMode;

typedef struct {
  byte hTotal, endHDisplay, startHBlanking, startHRetrace;
  VGAReg vTotal, endVDisplay, startVBlanking, endVBlanking, startVRetrace;
  EndHBlanking endHBlanking;
  EndHRetrace endHRetrace;
  EndVRetrace endVRetrace;
  byte offset;

  UnderlineLocation underlineLocation;
  MaxScanLine maxScanLine;
  PresetRowScan presetRowScan;
  Overflow overflow;

  CRTCMode crtcMode;
} PACKED CRTCRegs;

void getCRTCRegs(CRTCRegs* regs);
void setCRTCRegs(CRTCRegs* regs);

#endif
