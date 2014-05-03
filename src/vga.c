#include "vga.h"
#include "memory.h"

#define VGA_ADDR 0x3ce
#define VGA_DATA 0x3cf
#define SEQ_ADDR 0x3c4
#define SEQ_DATA 0x3c5
#define ACR_ADDR 0x

#define SEQ_RESET 0
#define SEQ_CLOCKING 1
#define SEQ_MAPMASK 2
#define SEQ_CHARMAP 3
#define SEQ_SEQMODE 4

typedef struct {
  word addr,index,data;
} VGAReg;

VGAReg bitmask = { VGA_ADDR, 8, VGA_DATA };

typedef struct {
  byte plane0:1;
  byte plane1:1;
  byte plane2:1;
  byte plane3:1;
} PACKED SetReset;
VGAReg setReset = { VGA_ADDR, 0, VGA_DATA };
VGAReg enableSetReset = { VGA_ADDR, 1, VGA_DATA };
VGAReg dontCare = { VGA_ADDR, 7, VGA_DATA };
VGAReg mapMask = { SEQ_ADDR, 2, SEQ_DATA };

typedef struct {
  byte colorCompare:4;
} PACKED ColorCompare;
VGAReg colorCompare = { VGA_ADDR, 2, VGA_DATA };

#define OP_ID 0
#define OP_AND 1
#define OP_OR 2
#define OP_XOR 3
typedef struct {
  byte rotateCount:3;
  byte operation:2;
} PACKED DataRotate;
VGAReg dataRotate = { VGA_ADDR, 3, VGA_DATA };

typedef struct {
  byte readMap:2;
} PACKED ReadMap;
VGAReg readMap = { VGA_ADDR, 4, VGA_DATA };

typedef struct {
  byte writeMode:2;
  byte _1:1;
  byte readMode:1;
  byte hostOE:1;
  byte shiftReg:1;
  byte shift256:1;
} PACKED GraphicsMode;
VGAReg graphicsMode = { VGA_ADDR, 5, VGA_DATA };

typedef struct {
  byte graphicsMode:1;
  byte chainOE:1;
  byte memoryMap:2;
} PACKED MiscReg;
VGAReg misc = { VGA_ADDR, 6, VGA_DATA };

typedef struct {
  SetReset setReset, enableSetReset;
  ColorCompare colorComp;
  DataRotate dataRotate;
  ReadMap readMap;
  GraphicsMode mode;
  MiscReg misc;
  SetReset dontCare;
  byte bitmask;
  SetReset mapMask;
} PACKED GraphicsRegs;

byte getVGAReg(VGAReg* r) {
  outportb(r->addr,r->index);
  return inportb(r->data);
}
void setVGAReg(VGAReg* r,byte b) {
  outportb(r->addr,r->index);
  outportb(r->data,b);
}

#define SETREG(x,r) setVGAReg(&(r),AS(byte,x))
#define GETREG(x,r) AS(byte,x) = getVGAReg(&(r))

void getGraphicsRegs(GraphicsRegs* r) {
  GETREG(r->setReset,setReset);
  GETREG(r->enableSetReset,enableSetReset);
  GETREG(r->colorComp,colorCompare);
  GETREG(r->dataRotate,dataRotate);
  GETREG(r->readMap,readMap);
  GETREG(r->mode,graphicsMode);
  GETREG(r->misc,misc);
  GETREG(r->dontCare,dontCare);
  GETREG(r->bitmask,bitmask);
}
void setGraphicsRegs(GraphicsRegs* r) {
  SETREG(r->setReset,setReset);
  SETREG(r->enableSetReset,enableSetReset);
  SETREG(r->colorComp,colorCompare);
  SETREG(r->dataRotate,dataRotate);
  SETREG(r->readMap,readMap);
  SETREG(r->mode,graphicsMode);
  SETREG(r->misc,misc);
  SETREG(r->dontCare,dontCare);
  SETREG(r->bitmask,bitmask);
}

GraphicsRegs curRegs;
GraphicsRegs oldRegs;

#define VGA_START 0xA0000
#define VGA_END 0xB0000
void switchMode() {
  GraphicsRegs old = oldRegs;
  oldRegs = curRegs;
  curRegs = old;
  setGraphicsRegs(&curRegs);
  dword* cur = VGA_START;
  for(;cur<VGA_END;cur++)
    *cur = 0xffff0000;
    /* switch((dword)cur&0xf) { */
    /* case 0: */
    /*   *cur = 0xffffffff; */
    /*   break; */
    /* case 4: */
    /*   *cur=0x40404444; */
    /*   break; */
    /* case 8: */
    /*   *cur=0x88888888; */
    /*   break; */
    /* case 12: */
    /*   *cur=0xcccccccc; */
    /*   break; */
    /* } */
}

static void initVGA() {
  getGraphicsRegs(&oldRegs);

  oldRegs.misc.graphicsMode = 1;
  oldRegs.misc.memoryMap = 1;
  oldRegs.mode.writeMode = 0;
  oldRegs.mode.shiftReg = 1;
  oldRegs.dataRotate.operation = OP_ID;
  oldRegs.dataRotate.rotateCount = 0;
  oldRegs.bitmask = 0xff;
  AS(byte,oldRegs.enableSetReset) = 0;
  AS(byte,oldRegs.mapMask) = 0xf;

  getGraphicsRegs(&curRegs);
}
Feature _vga_ = {
  .state = DISABLED,
  .label = "VGA",
  .initialize = &initVGA
};
