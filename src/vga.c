#include "vga.h"
#include "memory.h"

typedef struct {
  byte plane0:1;
  byte plane1:1;
  byte plane2:1;
  byte plane3:1;
} PACKED SetReset;

typedef struct {
  byte colorCompare:4;
} PACKED ColorCompare;

#define OP_ID 0
#define OP_AND 1
#define OP_OR 2
#define OP_XOR 3
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

typedef struct {
  SetReset setReset, enableSetReset;
  ColorCompare colorComp;
  DataRotate dataRotate;
  ReadMap readMap;
  GraphicsMode mode;
  MiscReg misc;
} PACKED GraphicsRegs;

#define REG_SETRESET 0
#define REG_ENABLESR 1
#define REG_CCOMP    2
#define REG_DATAROT  3
#define REG_READMAP  4
#define REG_MODE     5
#define REG_MISC     6
void getGraphicsRegs(GraphicsRegs* r) {
  outportb(VGA_ADDR,REG_SETRESET);
  AS(byte,r->setReset) = inportb(VGA_DATA);
  outportb(VGA_ADDR,REG_ENABLESR);
  AS(byte,r->enableSetReset) = inportb(VGA_DATA);
  outportb(VGA_ADDR,REG_CCOMP);
  AS(byte,r->colorComp) = inportb(VGA_DATA);
  outportb(VGA_ADDR,REG_DATAROT);
  AS(byte,r->dataRotate) = inportb(VGA_DATA);
  outportb(VGA_ADDR,REG_READMAP);
  AS(byte,r->readMap) = inportb(VGA_DATA);
  outportb(VGA_ADDR,REG_MODE);
  AS(byte,r->mode) = inportb(VGA_DATA);
  outportb(VGA_ADDR,REG_MISC);
  AS(byte,r->misc) = inportb(VGA_DATA);
}

static void initVGA() {
  
}
Feature _vga_ = {
  .state = DISABLED,
  .label = "VGA",
  .initialize = &initVGA
};
