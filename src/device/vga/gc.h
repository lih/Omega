#ifndef INCLUDED_VGA_GC
#define INCLUDED_VGA_GC

#include <device/vga/common.h>

typedef struct {
  byte colorCompare:4;
} PACKED ColorCompare;
typedef struct {
  byte rotateCount:3;
  byte operation:2;
} PACKED DataRotate;
typedef struct {
  byte readMap:2;
} PACKED ReadMap;
typedef struct {
  byte writeMode:2;
  byte _1:1;
  byte readMode:1;
  byte hostOE:1;
  byte shiftReg:1;
  byte shift256:1;
} PACKED GraphicsMode;
typedef struct {
  byte graphicsMode:1;
  byte chainOE:1;
  byte memoryMap:2;
} PACKED MiscReg;

#define OP_ID 0
#define OP_AND 1
#define OP_OR 2
#define OP_XOR 3

typedef struct {
  SetReset setReset, enableSetReset, dontCare;
  ColorCompare colorCompare;
  DataRotate dataRotate;
  ReadMap readMap;
  GraphicsMode graphicsMode;
  MiscReg misc;
  byte bitmask;
} PACKED GCRegs;

void getGCRegs(GCRegs* regs);
void setGCRegs(GCRegs* regs);

#endif
