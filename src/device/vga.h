#ifndef INCLUDED_VGA
#define INCLUDED_VGA

#include <core/feature.h>
#include <device/vga/crtc.h>
#include <device/vga/sequencer.h>
#include <device/vga/attributes.h>
#include <device/vga/gc.h>

extern Feature _vga_;

typedef struct {
  GCRegs gc;
  SeqRegs seq;
  CRTCRegs crtc;
  ACRRegs acr;
  MiscOutput miscOutput;
} PACKED VGARegs;

extern VGARegs mode_13h;

void switchMode();
void mode13h();

#endif
