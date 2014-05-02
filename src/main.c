#include "constants.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "acpi.h"
#include "feature.h"
#include "timer.h"
#include "process.h"
#include "syscall.h"
#include "init.h"

int running = 0; 

#define RED charMode = 0x0c;
#define GREEN charMode = 0x02;
#define CLEAR charMode = 0x0b;

void main () {
  clearFB();

  CLEAR printf("Loaded %d sectors (%d bytes) for the kernel\n",(KERNEL_SIZE+0x1ff)>>9,KERNEL_SIZE);
  RED printf("Initializing Omega: \n");

  GREEN;
  require(&_keyboard_);
  require(&_acpi_);
  require(&_process_);
  require(&_timer_);

  RED printf("The system is operational ! (Press Ctrl+Alt+Escape to shutdown)\n");

  CLEAR;

  enableInterrupts();
  syscall_spark(initUniv->index,&init);

  while(1) nop();
}




