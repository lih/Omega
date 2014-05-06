#include <device/vga.h>
#include <cpu/memory.h>
#include <device/framebuffer.h>

VGAReg miscOutput = REG(MISC,0);

void setVGAColor(int index, byte r, byte g, byte b) {
  outportb(COLOR_ADDR,index);
  outportb(COLOR_DATA,r);
  outportb(COLOR_DATA,g);
  outportb(COLOR_DATA,b);
}
void getVGARegs(VGARegs* r) {
  GETREG(r,miscOutput);
  getSeqRegs(&(r->seq));
  getCRTCRegs(&(r->crtc));
  getGCRegs(&(r->gc));
  getACRRegs(&(r->acr));
}
void setVGARegs(VGARegs* r) {
  SETREG(r,miscOutput);
  setSeqRegs(&(r->seq));
  setCRTCRegs(&(r->crtc));
  setGCRegs(&(r->gc));
  setACRRegs(&(r->acr));
}

#define VGA_START 0xA0000
#define VGA_END 0xC0000
void switchMode() {
  VGARegs old = oldRegs;
  oldRegs = curRegs;
  curRegs = old;
  
  setVGARegs(&curRegs);

  dword* cur = VGA_START;
  for(;cur<VGA_END;cur++)
    *cur = 0xffff0000;
}

static void initVGA() {
  byte reg = getVGAReg(&miscOutput);
  AS(MiscOutput,reg).ioas = 1;
  setVGAReg(&miscOutput,reg);

  getVGARegs(&oldRegs);

  oldRegs.gc.misc.graphicsMode = 1;
  oldRegs.gc.misc.memoryMap = 1;
  oldRegs.gc.graphicsMode.writeMode = 0;
  oldRegs.gc.graphicsMode.shiftReg = 1;
  oldRegs.gc.dataRotate.operation = OP_ID;
  oldRegs.gc.dataRotate.rotateCount = 0;
  oldRegs.gc.bitmask = 0xff;
  AS(byte,oldRegs.gc.enableSetReset) = 0;
  AS(byte,oldRegs.seq.mapMask) = 0xf;

  getVGARegs(&curRegs);
}
Feature _vga_ = {
  .state = DISABLED,
  .label = "VGA",
  .initialize = &initVGA
};

#define VGAFREQ 25000000
void mode13h() {
  printf("Mode 13h unimplemented...\n");
}
