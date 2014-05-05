#include <cpu/interrupt.h>
#include <cpu/syscall.h>
#include <cpu/descriptors.h>
#include <core/schedule.h>

Selector sysGate;

SyscallHandler syscalls[SYS_COUNT] = { 
  [SYS_WARP] = sys_warp,
  [SYS_SPARK] = sys_spark,
  [SYS_DIE] = sys_die,
  [SYS_ALLOC] = sys_alloc,
  [SYS_ACQUIRE] = sys_acquire,
  [SYS_RELEASE] = sys_release,
  [SYS_MAPTO] = sys_mapTo,
  [SYS_MAPFROM] = sys_mapFrom,
  [SYS_SPAWN] = sys_spawn,
  [SYS_ANIHILATE] = sys_anihilate,
  [SYS_WAIT] = sys_wait
};

void handleSyscalls() {
  TSS* ss = TSS_AT(getTaskRegister());

  while(1) {
    Task* state = getTask(ss->previousTask);
    dword scnum = state->tss->eax;
    SyscallHandler f;

    if(scnum<16 && (f = syscalls[scnum]) != 0) 
      f(state);
    else
      printf("Unhandled syscall %d\n",scnum);

    scheduleNext(ss);

    asm __volatile__ ( "iret" );
  }
}
static void initialize() {
  require(&_schedule_);
  require(&_irqs_);

  TSS* systss = SYS_STACK - sizeof(TSS);
  *systss = tss(kernelSpace.pageDir,handleSyscalls,SYS_STACK - sizeof(TSS));
  sysGate = addDesc(&gdt,tssDesc(systss,0));

  flushGDT();
  idts[48] = taskGate(sysGate,3);
}
Feature _syscalls_ = {
  .state = DISABLED,
  .label = "syscalls",
  .initialize = &initialize
};
