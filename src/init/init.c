#include <init/init.h>
#include <device/framebuffer.h>
#include <core/syscall.h>
#include <device/keyboard.h>
#include <device/vga.h>
#include <init/ping.h>
#include <meXa.h>
#include <cpu/pervasives.h>

#define TRI(x) (x + (x<<2) + (x<<4))
#define DUB(x) (x + ((x&0xc)<<2))
void palette() {
  int i;
  outportb(COLOR_ADDR,0);
  for(i=0;i<256;i++) {
    byte r = i >> 6, g = (i>>2)&0xf, b = i & 3;
    outportb(COLOR_DATA,TRI(r));
    outportb(COLOR_DATA,DUB(g));
    outportb(COLOR_DATA,TRI(b));
  };
}

byte color = 0xc0;
Semaphore colorSem = 0;

void vgaTest() {
  while(1) {
    dword* cur;
    dword col = color + (color << 8) + (color << 16) + (color << 24);
    for(cur=(dword*)VGA_START;cur<(dword*)VGA_END;cur++)
      *cur = col;
    syscall_acquire(&colorSem);
  }
}

void colorDial() {
  while(1) {
    int scan = readScan();
    switch(scan) {
    case 77: 
      color++;
      syscall_release(&colorSem,1);
      break;
    case 75:
      color--;
      syscall_release(&colorSem,1);
      break;
    }
  }
}

void pingPong() {
  syscall_spark(-1,&ping); 
  syscall_spark(-1,&pong); 
}
void init() {
  printf("Hello from ring %x !\n",(getPL() >> 12) & 3);
  
  syscall_spark(-1,&repl);
  syscall_spark(-1,&vgaTest);
  syscall_spark(-1,&colorDial);
  syscall_die();
}
