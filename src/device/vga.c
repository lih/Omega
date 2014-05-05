#include <device/vga.h>
#include <cpu/memory.h>

#define VGA_ADDR 0x3ce
#define VGA_DATA 0x3cf
#define SEQ_ADDR 0x3c4
#define SEQ_DATA 0x3c5
#define ACR_ADDR 0x3c0
#define ACR_DATA 0x3c1
#define CRT_ADDR 0x3d4
#define CRT_DATA 0x3d5
#define MISC_ADDR 0x3cc
#define MISC_DATA 0x3c2
#define INPUT_STATUS_1 0x3da
#define INPUT_STATUS_2 0x3c2

#define REG(t,i) { t##_ADDR , i , t##_DATA }

typedef struct {
  word addr,index,data;
} VGAReg;
VGAReg bitmask = REG(VGA,8);

typedef struct {
  byte plane0:1;
  byte plane1:1;
  byte plane2:1;
  byte plane3:1;
} PACKED SetReset;
VGAReg setReset = REG(VGA,0);
VGAReg enableSetReset = REG(VGA,1);
VGAReg dontCare = REG(VGA,7);
VGAReg mapMask = REG(SEQ,2);

typedef struct {
  byte colorCompare:4;
} PACKED ColorCompare;
VGAReg colorCompare = REG(VGA,2);
#define OP_ID 0
#define OP_AND 1
#define OP_OR 2
#define OP_XOR 3
typedef struct {
  byte rotateCount:3;
  byte operation:2;
} PACKED DataRotate;
VGAReg dataRotate = REG(VGA,3);

typedef struct {
  byte readMap:2;
} PACKED ReadMap;
VGAReg readMap = REG(VGA,4);

typedef struct {
  byte writeMode:2;
  byte _1:1;
  byte readMode:1;
  byte hostOE:1;
  byte shiftReg:1;
  byte shift256:1;
} PACKED GraphicsMode;
VGAReg graphicsMode = REG(VGA,5);

typedef struct {
  byte graphicsMode:1;
  byte chainOE:1;
  byte memoryMap:2;
} PACKED MiscReg;
VGAReg misc = REG(VGA,6);

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
VGAReg crtcMode = REG(CRT,0x17);
typedef struct { 
  byte vRetraceEnd:4;
  byte _:2;
  byte bandwidth:1;
  byte protect:1;
} PACKED VRetraceEnd;
VGAReg vRetraceEnd = REG(CRT,0x11);

typedef struct {
  byte ioas:1;
  byte ramEnable:1;
  byte clockSelect:2;
  byte _:1;
  byte oepage:1;
  byte hsyncp:1;
  byte vsyncp:1;
} PACKED MiscOutput;
VGAReg miscOutput = REG(MISC,0);

typedef struct {
  SetReset setReset, enableSetReset, mapMask;
  ColorCompare colorCompare;
  DataRotate dataRotate;
  ReadMap readMap;
  GraphicsMode graphicsMode;
  MiscReg misc;
  SetReset dontCare;
  byte bitmask;
  VRetraceEnd vRetraceEnd;
  MiscOutput miscOutput;
} PACKED GraphicsRegs;

byte getVGAReg(VGAReg* r) {
  switch(r->addr) {
  case ACR_ADDR:
    inportb(INPUT_STATUS_1);
    outportb(ACR_ADDR,r->index);
    return inportb(ACR_DATA);
  case MISC_ADDR:
    return inportb(MISC_ADDR);
  default:
    outportb(r->addr,r->index);
    return inportb(r->data);
  }
}
void setVGAReg(VGAReg* r,byte b) {
  switch(r->addr) {
  case ACR_ADDR:
    inportb(INPUT_STATUS_1);
    outportb(ACR_ADDR,r->index);
    outportb(ACR_ADDR,b);
    break;

  case MISC_ADDR:
    outportb(MISC_DATA,b);
    break;
    
  default:
    outportb(r->addr,r->index);
    outportb(r->data,b);
  }
}

#define SETREG(x,r) setVGAReg(&(r),AS(byte,(x)->r))
#define GETREG(x,r) AS(byte,(x)->r) = getVGAReg(&(r))

void getGraphicsRegs(GraphicsRegs* r) {
  byte reg = getVGAReg(&miscOutput);
  AS(MiscOutput,reg).ioas = 1;
  setVGAReg(&miscOutput,reg);
  reg = getVGAReg(&vRetraceEnd);
  AS(VRetraceEnd,reg).protect = 0;
  setVGAReg(&vRetraceEnd,reg);
  
  GETREG(r,setReset);
  GETREG(r,enableSetReset);
  GETREG(r,colorCompare);
  GETREG(r,dataRotate);
  GETREG(r,readMap);
  GETREG(r,graphicsMode);
  GETREG(r,misc);
  GETREG(r,dontCare);
  GETREG(r,bitmask);
}
void setGraphicsRegs(GraphicsRegs* r) {
  SETREG(r,setReset);
  SETREG(r,enableSetReset);
  SETREG(r,colorCompare);
  SETREG(r,dataRotate);
  SETREG(r,readMap);
  SETREG(r,graphicsMode);
  SETREG(r,misc);
  SETREG(r,dontCare);
  SETREG(r,bitmask);
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
  oldRegs.graphicsMode.writeMode = 0;
  oldRegs.graphicsMode.shiftReg = 1;
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
