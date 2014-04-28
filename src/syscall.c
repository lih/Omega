#include "interrupt.h"
#include "syscall.h"
#include "schedule.h"

void handleSyscalls() {
  Task* task = getTask();
  while(1) {
    Task* state = descBase(DESCRIPTOR_AT(gdt,task->tss.previousTask));
    dword scnum = state->tss.eax;
    SyscallHandler f;

    if(scnum<16 && (f = syscalls[scnum]) != 0) 
      f(state);
    else
      printf("Unhandled syscall %d\n",scnum);

    asm __volatile__ ( "iret" );
  }
}
static void initialize() {
  require(&_schedule_);
  
  Task* systss = newTask();
  systss->tss = tss(kernelSpace.pageDir,0,0,0,0,0,0);
  systss->tss.eip = handleSyscalls;
  systss->tss.esp = SYS_STACK;
  
  syscalls[SYS_WARP] = sys_warp;
  syscalls[SYS_SPAWN] = sys_spawn;
  syscalls[SYS_DIE] = sys_die;

  Selector sysdesc = addDesc(&gdt,tssDesc(systss,0,0));
  idts[48] = taskGate(sysdesc,0);
}
Feature _syscalls_ = {
  .state = DISABLED,
  .label = "syscalls",
  .initialize = &initialize
};
