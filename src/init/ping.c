#include <init/ping.h>
#include <cpu/syscall.h>
#include <device/framebuffer.h>

Semaphore pingsem = 1, pongsem = 0;
int nping = 0;
#define MAXPING 5

void ping() {
  int c = 1;
  while(c) {
    syscall_acquire(&pingsem);
    printf("Ping\n");
    nping++;
    if(nping >= MAXPING) {
      printf("I have no more ping in me...\n");
      c = 0;
    }
    else 
      syscall_wait(1,0);
    syscall_release(&pongsem,1);
  }
  syscall_die();
}
void pong() {
  int c = 1;
  while(c) {
    syscall_acquire(&pongsem);
    printf("Pong\n");
    nping++;
    if(nping >= MAXPING) {
      printf("I have no more pong in me...\n");
      c = 0;
    }
    syscall_wait(1,0);
    syscall_release(&pingsem,1);
  }
  syscall_die();
}

