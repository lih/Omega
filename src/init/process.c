#include <core/schedule.h>
#include <core/universe.h>
#include <constants.h>
#include <cpu/syscall.h>
#include <device/framebuffer.h>
#include <device/vga.h>
#include <cpu/interrupt.h>
#include <init/init.h>

Universe* initUniv;

#define MAP_STACK(u,s) mapPage(u,STACK_PAGE(s,0),STACK_PAGE(s,0),0)

static void initialize() {
  require(&_schedule_);
  require(&_syscalls_);

  initUniv = newUniverse(&kernelSpace);

  initUniv->dpl = 3;
  void* vpage;
  for(vpage = (void*)(KERNEL_START & 0xfffff000);vpage < (void*)&KERNEL_END;vpage+=PAGE_SIZE)
    mapPage(initUniv,vpage,vpage,1);
  for(vpage = (void*)(FB_MEM & 0xfffff000);vpage < (void*)FB_END;vpage+=PAGE_SIZE)
    mapPage(initUniv,vpage,vpage,1);
  for(vpage = (void*)(VGA_START & 0xfffff000);vpage < (void*)VGA_END;vpage+=PAGE_SIZE)
    mapPage(initUniv,vpage,vpage,1);
  MAP_STACK(initUniv,EXC_STACK);
  MAP_STACK(initUniv,INT_STACK);
  MAP_STACK(initUniv,SYS_STACK);
  MAP_STACK(initUniv,KEY_STACK);
  MAP_STACK(initUniv,IRQ_STACK);
}
Feature _process_ = {
  .state = DISABLED,
  .label = "main process",
  .initialize = initialize
};
