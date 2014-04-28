#include "descriptors.h"

Descriptor idts[256];

Selector addDesc(TablePtr* tbl,Descriptor desc) {
  int index = tbl->limit / sizeof(Descriptor);

  tbl->base[index] = desc;
  tbl->limit += sizeof(Descriptor);

  return index*sizeof(Descriptor);
}

void* descBase(Descriptor d) {
  return d.base_lo + (d.base_mi << 16) + (d.base_hi << 24);
}

TSS tss(Dir* pageDir,word ss0,dword esp0,
	word ss1,dword esp1,word ss2,dword esp2) {
  TSS ret = {
    .esp0 = esp0, .ss0 = ss0,
    .esp1 = esp1, .ss1 = ss1,
    .esp2 = esp2, .ss2 = ss2,
    .cs = CODE_SEGMENT, .ds = DATA_SEGMENT, .es = DATA_SEGMENT,
    .fs = DATA_SEGMENT, .gs = DATA_SEGMENT, .ss = DATA_SEGMENT,
    .cr3 = (dword)pageDir,
    .trap = 0
  };

  return ret;
}

Descriptor tssDesc(TSS* tss,byte dpl,byte busy) {
  dword base = tss;
  Descriptor ret = {
    .limit_lo = sizeof(TSS),
    .base_lo = base,
    .base_mi = base >> 16,
    .type = 9 | (busy?2:0),
    .codeOrData = 0,
    .dpl = dpl,
    .present = 1,
    .limit_hi = 0,
    .code64 = 0,
    .opsize = 1,
    .granularity = 1,
    .base_hi = base >> 24
  };
  return ret;
}
Descriptor taskGate(Selector tssSel,byte dpl) {
  Descriptor ret = {
    .present = 1,
    .dpl = dpl,
    .type = 5,
    .codeOrData = 0,
    .base_lo = tssSel
  };
  return ret;
}
Descriptor interruptGate(Selector cs,void* pbase,byte dpl) {
  dword base = pbase;
  
  Descriptor ret = {
    .limit_lo = base & 0xffff,
    .base_lo = cs,
    .type = 14,
    .codeOrData = 0,
    .dpl = dpl,
    .present = 1
  };

  ((word*)&ret)[3] = base >> 16;
    
  return ret;
}
