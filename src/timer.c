#include "timer.h"
#include "schedule.h"
#include "interrupt.h"

int millis = 0;
int seconds = 0;
int phase;

void setTimerFreq(int hz) {
  phase = FREQUENCY / hz;       
  outportb(0x43, 0x36);           /* Set our command byte 0x36 */
  outportb(0x40, phase & 0xff);
  outportb(0x40, phase >> 8); /* Set two-byte phase */
}

static void initialize() {
  require(&_schedule_);
  require(&_interrupts_);
  
  setTimerFreq(TIMER_FREQ);
  PAD printf("Set timer frequency to %dHz\n",TIMER_FREQ);
}
Feature _timer_ = {
  .state = DISABLED,
  .label = "timer",
  .initialize = &initialize
};

