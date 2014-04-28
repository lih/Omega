#include "acpi.h"
#include "framebuffer.h"

RSDP* rsdp;
FADT* fadt;

RSDP*    findRSDP();
SDTable* findSDTable(char*);

static void initACPI() {
  rsdp = findRSDP();
  fadt = (FADT*) findSDTable("FACP");
  
  if(fadt->smiCommandPort != 0
     && (fadt->acpiEnable == 0 || fadt->acpiDisable)) {
    /* Enable ACPI if it wasn't already initialized by the system */
    outportb(fadt->smiCommandPort,fadt->acpiEnable);
  }
}
Feature _acpi_ = {
  .state = DISABLED,
  .label = "ACPI",
  .initialize = &initACPI
};

void shutdown() {
  /* FIXME: Magic values only recognized by Bochs. */
  outportw(0xb004,0x2000);
}
RSDP* findRSDP() {
  byte rsdHeader[8] = { 'R', 'S', 'D', ' ', 'P', 'T', 'R', ' ' };
  dword* hdr = &rsdHeader;
  dword* i;

  for(i=0;i<4096;i+=(16/sizeof(*i)))
    if(i[0] == hdr[0] && i[1] == hdr[1])
      return i;
  for(i=0xe0000;i<0x100000;i+=16/(sizeof(*i)))
    if(i[0] == hdr[0] && i[1] == hdr[1])
      return i;

  printf("ACPI header not found.\n");
}
SDTable* findSDTable(char* sig) {
  SDTable* rsdt = rsdp->rsdt;
  SDTable** sdts = ((void*)rsdt) + sizeof(*rsdt) ;
  int len = (rsdt->length - sizeof(*rsdt)) / sizeof(*sdts);

  int i;
  for(i=0;i<len;i++) {
    if(AS(dword,sdts[i]->signature) == AS(dword,*sig))
      return &sdts[i];
  }
      
  return 0;
}
