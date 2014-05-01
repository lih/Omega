#include "interrupt.h"
#include "syscall.h"
#include "schedule.h"
#include "descriptors.h"

Selector sysGate;

void handleSyscalls() {
  TSS* ss = TSS_AT(getTaskRegister());

  disableInterrupts();
  while(1) {
    Task* state = getTask(ss->previousTask);
    dword scnum = state->tss->eax;
    SyscallHandler f;

    if(scnum<16 && (f = syscalls[scnum]) != 0) 
      f(state);
    else
      printf("Unhandled syscall %d\n",scnum);

    DESCRIPTOR_AT(gdt,ss->previousTask) = tssDesc(state->tss,state->univ->dpl,1);
    flushGDT();

    asm __volatile__ ( "iret" );
  }
}
static void initialize() {
  require(&_schedule_);
  require(&_irqs_);

  syscalls[SYS_WARP] = sys_warp;
  syscalls[SYS_SPAWN] = sys_spawn;
  syscalls[SYS_DIE] = sys_die;
  syscalls[SYS_ALLOC] = sys_alloc;

  TSS* systss = SYS_STACK - sizeof(TSS);
  *systss = tss(kernelSpace.pageDir,handleSyscalls,SYS_STACK - sizeof(TSS));
  sysGate = addDesc(&gdt,tssDesc(systss,0,0));

  flushGDT();
  idts[48] = taskGate(sysGate,0);
}
Feature _syscalls_ = {
  .state = DISABLED,
  .label = "syscalls",
  .initialize = &initialize
};
