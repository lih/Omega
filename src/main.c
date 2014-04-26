#include "constants.h"
#include "memory.h"
#include "framebuffer.h"
#include "interrupt.h"
#include "keyboard.h"
#include "universe.h"
#include "schedule.h"
#include "pervasives.h"
#include "acpi.h"

int running = 0; 

void handleKey(IDTParams* _) {
  int scan = inportb(0x60);
  
  if(!(scan & 0x80)) {
    switch(layout[scan]) {
    case 0:
      printf("Unassociated scancode %d\n",scan);
      break;
      
    case 27: /* ESCAPE */
      running = 0;
      break;

    default:
      putChar(layout[scan]);
    }
  }
} 

void main () {
  clearFB();
  printf("Initializing Omega: \n");
  printf("  - Kernel size: %d sectors\n",ksize>>9);

  initACPI();
  printf("  - Enabled ACPI\n");
  initMemory();
  printf("  - Detected %dkB of RAM (%d 4kB pages)\n",(PAGE_SIZE*page_count)/1024,page_count);
  initInterrupts();
  printf("  - Initialized interrupts and exceptions\n");
  irqs[1] = handleKey;
  printf("  - Initialized keyboard echo driver\n");
  initUniverse();
  printf("  - Initialized universe\n");
  initSchedule();
  printf("  - Initialized scheduling\n");
  setTimerFreq(118);
  printf("  - Initialized timer with phase %d\n",timerPhase);

  running = 1;
  while(running) {
    nop();
  }

  printf("Shutting down Omega after %d seconds %d milliseconds...\n",seconds,millis);
  shutdown();
}




