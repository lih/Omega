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
VGAReg hTotal = REG(CRT,0);
VGAReg endHDisplay = REG(CRT,1);
VGAReg startHBlanking = REG(CRT,2);
VGAReg startHRetrace = REG(CRT,4);
VGAReg vTotal = REG(CRT,6);
VGAReg startVRetrace = REG(CRT,0x10);
VGAReg endVDisplay = REG(CRT,0x12);
VGAReg offsetReg = REG(CRT,0x13);
VGAReg startVBlanking = REG(CRT,0x15);
VGAReg endVBlanking = REG(CRT,0x16);
VGAReg overscanIndex = REG(ACR,0x31);

typedef struct {
  byte underlineLocation:5;
  byte div4:1;
  byte dw:1;
} PACKED UnderlineLocation;
VGAReg underlineLocation = REG(CRT,0x14);
typedef struct {
  byte _:1;
  byte extmem:1;
  byte oedis:1;
  byte chain4:1;
} PACKED MemoryMode;
VGAReg memoryMode = REG(SEQ,4);
typedef struct {
  byte _98dm:1;
  byte _:1;
  byte slr:1;
  byte s4:1;
  byte sd:1;
} PACKED ClockingMode;
VGAReg clockingMode = REG(SEQ,1);
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
VGAReg attributeMode = REG(ACR,0x30);
typedef struct {
  byte maxScanLine:5;
  byte svb9:1;
  byte lc9:1;
  byte sd:1;
} PACKED MaxScanLine;
VGAReg maxScanLine = REG(CRT,9);
typedef struct {
  byte presetRowScan:5;
  byte panning:2;
} PACKED PresetRowScan;
VGAReg presetRowScan = REG(CRT,8);
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
VGAReg overflow = REG(CRT,7);
typedef struct {
  byte endHBlanking:5;
  byte displaySkew:2;
  byte evra:1;
} PACKED EndHBlanking;
VGAReg endHBlanking = REG(CRT,3);
typedef struct {
  byte endHRetrace:5;
  byte hSkew:2;
  byte ehb5:1;
} PACKED EndHRetrace;
VGAReg endHRetrace = REG(CRT,5);

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
VGAReg colorPlaneEnable = REG(ACR,0x32);

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
  byte endVRetrace:4;
  byte _:2;
  byte bandwidth:1;
  byte protect:1;
} PACKED EndVRetrace;
VGAReg endVRetrace = REG(CRT,0x11);

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
  MiscOutput miscOutput;
  byte hTotal,vTotal,
    startHBlanking,startVBlanking,
    startHRetrace,startVRetrace,
    endVDisplay,endHDisplay;
  byte endVBlanking; EndHBlanking endHBlanking;
  EndVRetrace endVRetrace; EndHRetrace endHRetrace;
  Overflow overflow;
} PACKED VGARegs;

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

void getVGARegs(VGARegs* r) {
  byte reg = getVGAReg(&miscOutput);
  AS(MiscOutput,reg).ioas = 1;
  setVGAReg(&miscOutput,reg);
  reg = getVGAReg(&endVRetrace);
  AS(EndVRetrace,reg).protect = 0;
  setVGAReg(&endVRetrace,reg);
  
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
void setVGARegs(VGARegs* r) {
  byte reg = getVGAReg(&miscOutput);
  AS(MiscOutput,reg).ioas = 1;
  setVGAReg(&miscOutput,reg);
  reg = getVGAReg(&endVRetrace);
  AS(EndVRetrace,reg).protect = 0;
  setVGAReg(&endVRetrace,reg);

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

VGARegs curRegs;
VGARegs oldRegs;

#define VGA_START 0xA0000
#define VGA_END 0xB0000
void switchMode() {
  VGARegs old = oldRegs;
  oldRegs = curRegs;
  curRegs = old;
  setVGARegs(&curRegs);
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
  getVGARegs(&oldRegs);

  oldRegs.misc.graphicsMode = 1;
  oldRegs.misc.memoryMap = 1;
  oldRegs.graphicsMode.writeMode = 0;
  oldRegs.graphicsMode.shiftReg = 1;
  oldRegs.dataRotate.operation = OP_ID;
  oldRegs.dataRotate.rotateCount = 0;
  oldRegs.bitmask = 0xff;
  AS(byte,oldRegs.enableSetReset) = 0;
  AS(byte,oldRegs.mapMask) = 0xf;

  getVGARegs(&curRegs);
}
Feature _vga_ = {
  .state = DISABLED,
  .label = "VGA",
  .initialize = &initVGA
};

#define VGAFREQ 25000000
void mode13h() {
  byte reg = getVGAReg(&miscOutput);
  AS(MiscOutput,reg).ioas = 1;
  setVGAReg(&miscOutput,reg);
  reg = getVGAReg(&endVRetrace);
  AS(EndVRetrace,reg).protect = 0;
  setVGAReg(&endVRetrace,reg);

  setVGAReg(&attributeMode,0x41);
  setVGAReg(&colorPlaneEnable,0xf);
  setVGAReg(&clockingMode,1);
  setVGAReg(&memoryMode,0xe);
  setVGAReg(&graphicsMode,0x40);
  setVGAReg(&misc,5);

  setVGAReg(&overflow,0x1f);
  setVGAReg(&maxScanLine,0x41);
  setVGAReg(&underlineLocation,0x40);

  setVGAReg(&crtcMode,0xa3);


  setVGAReg(&hTotal,0x5f);
  setVGAReg(&endHDisplay,0x4f);
  setVGAReg(&startHBlanking,0x50);
  setVGAReg(&endHBlanking,0x82);
  setVGAReg(&startHRetrace,0x54);
  setVGAReg(&endHRetrace,0x80);

  setVGAReg(&vTotal,0xbf);
  setVGAReg(&endVDisplay,0x8f);
  setVGAReg(&startVBlanking,0x96);
  setVGAReg(&endVBlanking,0xb9);
  setVGAReg(&startVRetrace,0x9c);
  setVGAReg(&endVRetrace,0x8e);

  setVGAReg(&miscOutput,0x63);
}
