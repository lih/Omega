#include "descriptors.h"

word addDescriptor(Descriptor desc) {
  int index = gdtDesc.limit / sizeof(Descriptor);

  gdtDesc.base[index] = desc;
  gdtDesc.limit += sizeof(Descriptor);

  flushGDT();

  return index*sizeof(Descriptor);
}

void* descBase(Descriptor d) {
  return d.base_lo + (d.base_mi << 16) + (d.base_hi << 24);
}
