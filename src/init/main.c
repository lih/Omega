#include <constants.h>
#include <core/feature.h>
#include <cpu/interrupt.h>
#include <cpu/pervasives.h>
#include <core/syscall.h>
#include <device/acpi.h>
#include <device/framebuffer.h>
#include <device/keyboard.h>
#include <device/timer.h>
#include <device/vga.h>
#include <init/init.h>
#include <init/process.h>

int running = 0; 

#define RED charMode = 0x0c;
#define GREEN charMode = 0x02;
#define CLEAR charMode = 0x0b;

int main () {
  clearFB();

  CLEAR printf("Loaded %d sectors (%d bytes) for the kernel\n",(KERNEL_SIZE+0x1ff)>>9,KERNEL_SIZE);
  RED printf("Initializing Omega: \n");

  GREEN;
  require(&_keyboard_);
  require(&_acpi_);
  require(&_process_);
  require(&_timer_);
  require(&_vga_);

  RED printf("The system is operational ! (Press Ctrl+Alt+Escape to shutdown)\n");

  CLEAR;

  enableInterrupts();
  syscall_spark(initUniv->index,&init);

  while(1) nop();

  return 0;
}




