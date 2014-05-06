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

VGARegs fbMode;
static void initVGA() {
  MODREG(miscOutput,MiscOutput,REG.ioas = 1);

  getVGARegs(&fbMode);
}
Feature _vga_ = {
  .state = DISABLED,
  .label = "VGA",
  .initialize = &initVGA
};

VGARegs* oldRegs = &fbMode;
VGARegs* curRegs = &fbMode;
void switchMode() {
  VGARegs* old = oldRegs;
  oldRegs = curRegs;
  curRegs = old;
  
  setVGARegs(curRegs);

  dword* cur = VGA_START;
  for(;cur<VGA_END;cur++)
    *cur = 0xffff0000;
}

void mode13h() {
  setVGARegs(&mode_13h);
}

VGARegs mode_13h = {
  .seq = {
    .clockingMode = {
      ._98dm = 1
    },
    .memoryMode = {
      .extmem = 1,
      .chain4 = 1
    },
    .mapMask = { 1,1,1,1 }
  },
  .acr = {
    .attributeMode = {
      .atge = 1,
      ._8bit = 1
    },
    .colorPlaneEnable = { 1,1,1,1 }
  },
  .crtc = {
    .hTotal = 0x5f,
    .endHDisplay = 0x4f,
    .startHBlanking = 0x50,
    .startHRetrace = 0x54,
    .endHBlanking = {
      .evra = 1,
      .endHBlanking = 2
    },
    .endHRetrace = {
      .endHRetrace = 0,
      .ehb5 = 1
    },
    
    .vTotal = 0xbf,
    .endVDisplay = 0x8f,
    .startVBlanking = 0x96,
    .startVRetrace = 0x9c,
    .endVBlanking = 0xb9,
    .endVRetrace = { .endVRetrace = 0xe },
    
    .underlineLocation = {  },
    .maxScanLine = {
      .maxScanLine = 1,
      .lc9 = 1
    },
    .overflow = { 0,0,0,1,1,1,1,1 },
    .crtcMode = {
      .se = 1, .aw = 1,
      .map13 = 1, .map14 = 1,
    },
  },
  .gc = {
    .bitmask = 0xff,
    .graphicsMode = { .shift256 = 1 },
    .misc = {
      .graphicsMode = 1, .memoryMap = 1
    },
  },
  .miscOutput = {
    .ioas = 1, .ramEnable = 1,
    .hsyncp = 1, .oepage = 1
  }
};
