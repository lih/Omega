#include <cpu/interrupt.h>
#include <core/syscall.h>
#include <cpu/descriptors.h>
#include <core/schedule.h>
#include <cpu/pervasives.h>
#include <device/framebuffer.h>

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
  [SYS_WAIT] = sys_wait,
  [SYS_BIOS] = sys_bios
};

BIOSFun biosFuns[] = {
  bios_mode03h, bios_mode13h
};

void sys_bios(Task* t) {
  dword ind = t->tss->ebx;

  if(ind < SZ(biosFuns)) 
    realMode(biosFuns[ind]);
}
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

  TSS* systss = (void*)SYS_STACK - sizeof(TSS);
  *systss = tss(kernelSpace.pageDir,handleSyscalls,systss);
  sysGate = addDesc(&gdt,tssDesc(systss,0));

  flushGDT();
  idts[48] = taskGate(sysGate,3);
}
Feature _syscalls_ = {
  .state = DISABLED,
  .label = "syscalls",
  .initialize = &initialize
};
