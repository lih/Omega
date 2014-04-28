#include "constants.h"
#include "memory.h"
#include "framebuffer.h"
#include "interrupt.h"
#include "keyboard.h"
#include "universe.h"
#include "schedule.h"
#include "pervasives.h"
#include "acpi.h"
#include "feature.h"

int running = 0; 

void main () {
  clearFB();

  charMode = 0x0c;
  printf("Initializing Omega: \n");

  printf("taskRegister: %x\n",setTaskRegister);

  charMode = 0x02;
  initFeature();
  printf("Loaded kernel of size %dB (%d sectors)\n",KERNEL_SIZE,(KERNEL_SIZE+0x1ff)>>9);
  require(ACPI);
  require(UNIVERSE);  
  require(SCHEDULE);
  require(INTERRUPTS);
  require(KEYBOARD);
  require(SYSCALLS);
  setCursor();

  charMode = 0x0b;

  running = 1;
  while(running) {
    nop();
  }

  printf("Shutting down Omega after %d seconds %d milliseconds...\n",seconds,millis);
  shutdown();
}




