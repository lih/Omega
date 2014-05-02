#include "init.h"
#include "framebuffer.h"
#include "syscall.h"

dword pingsem = 1;
dword pongsem = 0;

void ping() {
  while(1) {
    syscall_acquire(&pingsem);
    printf("Ping\n");
    syscall_wait(1,0);
    syscall_release(&pongsem,1);
  }
}
void pong() {
  while(1) {
    syscall_acquire(&pongsem);
    printf("Pong\n");
    syscall_wait(1,0);
    syscall_release(&pingsem,1);
  }
}

void init() {
  printf("Hello from ring %x !\n",(getPL() >> 12) & 3);
  
  syscall_spark(-1,&ping);
  syscall_spark(-1,&pong);
  syscall_die();
}
