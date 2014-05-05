#include <init/init.h>
#include <device/framebuffer.h>
#include <cpu/syscall.h>
#include <device/keyboard.h>

dword pingsem = 1, pongsem = 0;
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

byte readChar() {
  syscall_acquire(&keyboardSem);
  return currentScan;
}
int readn(int n, char limit, byte* buf) {
  int i;
  for(i=0;i<n;i++) {
    buf[i] = readChar();
    if(buf[i] == limit)
      break;
  }
  buf[i] = '\0';
  return i;
}
void testRead() {
  char buf[80];
  while(1) {
    int n = readn(79,'\n',buf);
    printf("Read %d chars: %s\n",n,buf);
  }
  syscall_die();
}

void init() {
  printf("Hello from ring %x !\n",(getPL() >> 12) & 3);
  
  syscall_spark(-1,&ping); 
  syscall_spark(-1,&pong); 
  syscall_spark(-1,&testRead);
  syscall_die();
}
