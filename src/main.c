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

int running = 0; 

#define RED charMode = 0x0c;
#define GREEN charMode = 0x02;
#define CLEAR charMode = 0x0b;

void main () {
  clearFB();

  RED printf("Initializing Omega: \n");

  GREEN;
  printf("- Loaded kernel of size %dB (%d sectors)\n",KERNEL_SIZE,(KERNEL_SIZE+0x1ff)>>9);
  require(&_timer_);
  require(&_keyboard_);
  require(&_acpi_);

  RED printf("The system is operational !\n");

  CLEAR;

  running = 1;
  while(running) {
    nop();
  }

  printf("Shutting down Omega after %d seconds %d milliseconds...\n",seconds,millis);
  shutdown();
}




