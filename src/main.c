#include "constants.h"
#include "memory.h"
#include "framebuffer.h"
#include "interrupt.h"

void main () {
  clear();
  printf("Initializing Omega: \n");
  init_memory();
  printf("  - Initialized %d pages of size %d for a total of %d bytes\n",page_count,PAGE_SIZE,PAGE_SIZE*page_count);
  init_interrupts();
  printf("  - Initialized interrupts and exceptions");
  while(1) {
    ;
  }
}

