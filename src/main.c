#include "constants.h"
#include "memory.h"
#include "framebuffer.h"
#include "interrupt.h"

void main () {
  clear();
  printf("Initializing Omega: \n");
  init_memory();
  printf("  - Initialized %d pages of size %d\n",page_count,PAGE_SIZE);
  printf("  - IDT size : %d bytes (flags : %d)\n",sizeof(IDTEntry),sizeof(IDTFlags));
  init_interrupts();
  printf("  - Initialized interrupts");
}

