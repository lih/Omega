#include <cpu/descriptors.h>
#include <device/framebuffer.h>

Descriptor idts[256] = { { } };

Selector addDesc(TablePtr* tbl,Descriptor desc) {
  int index = (1+tbl->limit) / sizeof(Descriptor);

  tbl->base[index] = desc;
  tbl->limit += sizeof(Descriptor);
  
  return index*sizeof(Descriptor);
}

void* descBase(Descriptor d) {
  return (void*)(d.base_lo + (d.base_mi << 16) + (d.base_hi << 24));
}

TSS tss(Dir* pageDir,void* eip,void* esp) {
  TSS ret = {
    .eflags = { 
      ._one = 1, ._zero = 0, ._zero2 = 0, ._zero3 = 0, ._zero4 = 0,
      ._if = 0, .iopl = 0
    },
    .cs = CODE_SEGMENT, .ds = DATA_SEGMENT, .es = DATA_SEGMENT,
    .fs = DATA_SEGMENT, .gs = DATA_SEGMENT, .ss = DATA_SEGMENT,
    .cr3 = (dword)pageDir,
    .trap = 0,
    .eip = (dword)eip,
    .esp = (dword)esp
  };

  return ret;
}

Descriptor tssDesc(TSS* tss,byte busy) {
#define base ((dword)tss)
  Descriptor ret = {
    .limit_lo = sizeof(TSS)-1,
    .base_lo = base,
    .base_mi = base >> 16,
    .type = 9 | (busy?2:0),
    .codeOrData = 0,
    .dpl = 0,
    .present = 1,
    .limit_hi = 0,
    .code64 = 0,
    .opsize = 1,
    .granularity = 1,
    .base_hi = base >> 24
  };
  return ret;
#undef base
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
#define base ((dword)pbase)
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
#undef base
}
