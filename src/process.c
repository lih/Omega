#include "schedule.h"
#include "universe.h"
#include "constants.h"
#include "syscall.h"
#include "framebuffer.h"
#include "interrupt.h"

Universe* initUniv;

void nop();
static void main() {
  printf("Hello from ring 1 !\n");
  
  while(1) nop();
}
#define MAP_STACK(u,s) mapPage(u,STACK_PAGE(s,0),STACK_PAGE(s,0),0)

static void initialize() {
  require(&_schedule_);
  require(&_syscalls_);

  initUniv = newUniverse();

  initUniv->dpl = 1;
  void* vpage;
  for(vpage = KERNEL_START & 0xfffff000;vpage < &KERNEL_END;vpage+=PAGE_SIZE)
    mapPage(initUniv,vpage,vpage,1);
  for(vpage = FB_MEM & 0xfffff000;vpage < FB_END;vpage+=PAGE_SIZE)
    mapPage(initUniv,vpage,vpage,1);
  MAP_STACK(initUniv,EXC_STACK);
  MAP_STACK(initUniv,INT_STACK);
  MAP_STACK(initUniv,SYS_STACK);
  MAP_STACK(initUniv,KEY_STACK);
  MAP_STACK(initUniv,IRQ_STACK);
  
  syscall_spawn(initUniv,&main);
}
Feature _process_ = {
  .state = DISABLED,
  .label = "processes",
  .initialize = initialize
};
