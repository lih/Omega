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
  getGCRegs(&(r->gc));
  getACRRegs(&(r->acr));
  getCRTCRegs(&(r->crtc));
}
void setVGARegs(VGARegs* r) {
  disableScreen();

  SETREG(r,miscOutput);
  setACRRegs(&(r->acr));
  setCRTCRegs(&(r->crtc));
  setGCRegs(&(r->gc));
  setSeqRegs(&(r->seq));
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

VGARegs mode_13h = {
  .seq = {
    .clockingMode = {
      ._98dm = 1
    },
    .memoryMode = {
      .extmem = 1,
      .oedis = 1,
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
    
    .underlineLocation = { .dw = 1 },
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
    .graphicsMode = { .hostOE = 1, .shift256 = 1 },
    .misc = {
      .graphicsMode = 1, .memoryMap = 1
    },
  },
  .miscOutput = {
    .ioas = 1, .ramEnable = 1,
  }
};
