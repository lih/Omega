#ifndef INCLUDED_INTERRUPT
#define INCLUDED_INTERRUPT

#include "memory.h"

typedef struct {
  byte _fourteen:5;
  byte dpl:2;
  byte present:1;
} __attribute__((__packed__)) IDTFlags;
typedef struct {
  word base_lo;
  word sel;
  byte _zero;
  IDTFlags flags;
  word base_hi;
} __attribute((__packed__)) IDTEntry;

void enableInterrupts();
void disableInterrupts();

#endif
