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
#include "timer.h"
#include "process.h"

int running = 0; 

#define RED charMode = 0x0c;
#define GREEN charMode = 0x02;
#define CLEAR charMode = 0x0b;

void main () {
  clearFB();

  CLEAR printf("Loaded %d sectors (%d bytes) for the kernel\n",(KERNEL_SIZE+0x1ff)>>9,KERNEL_SIZE);
  RED printf("Initializing Omega: \n");

  GREEN;
  require(&_timer_);
  require(&_keyboard_);
  require(&_acpi_);
  require(&_process_);

  RED printf("The system is operational ! (Press Ctrl+Alt+Escape to shutdown)\n");

  CLEAR;

  enableInterrupts();

  running = 1;
  while(running) {
    nop();
  }

  printf("Shutting down Omega after %d seconds %d milliseconds...\n",seconds,millis);
  shutdown();
}




