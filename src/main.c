#include "constants.h"
#include "memory.h"
#include "framebuffer.h"
#include "interrupt.h"
#include "keyboard.h"

int running;
int millis = 0;
int seconds = 0;
int phase;
#define FREQUENCY 1193180
void timer(IDTParams* _) {
  millis += phase;
  if(millis >= FREQUENCY) {
    millis -= FREQUENCY;
    seconds++;
  }
}

void set_timer_phase(int hz) {
  phase = FREQUENCY / hz;       
  outportb(0x43, 0x36);           /* Set our command byte 0x36 */
  outportb(0x40, phase & 0xFF);   /* Set low byte of phase */
  outportb(0x40, phase >> 8);     /* Set high byte of phase */
}

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
  clear();
  printf("Initializing Omega: \n");
  init_memory();
  printf("  - Detected %dB of RAM (%d pages*%d bytes per page)\n",PAGE_SIZE*page_count,page_count,PAGE_SIZE);
  init_interrupts();
  printf("  - Initialized interrupts and exceptions\n");
  set_timer_phase(118);
  irqs[0] = timer;
  printf("  - Initialized timer with phase %d\n",phase);
  irqs[1] = handleKey;
  printf("  - Initialized echo driver\n");

  running = 1;
  while(running) {
    nop();
  }

  printf("Shutting down Omega...\n");
  shutdown();
}




