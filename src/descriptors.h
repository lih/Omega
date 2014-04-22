#ifndef INCLUDED_DESCRIPTORS
#define INCLUDED_DESCRIPTORS

#include "memory.h"

typedef struct {
  word limit_lo;
  word base_lo;
  byte base_mi;
  byte type:4;
  byte codeOrData:1;
  byte dpl:2;
  byte present:1;
  byte limit_hi:4;
  byte free:1;
  byte code64:1;		/* set for 64-bit code segment */
  byte opsize:1; 		/* set for 32-bit mode */
  byte granularity:1; 		/* set for big pages */
  byte base_hi;
} __attribute__((__packed__)) Descriptor;
typedef struct {
  word limit;
  Descriptor* base;
} __attribute__((__packed__)) TablePtr;

extern TablePtr gdtDesc;

word addDescriptor(Descriptor desc);
void* descBase(Descriptor desc);

#endif
