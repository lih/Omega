#include <device/framebuffer.h>
#include <util/memory.h>
#include <constants.h>
#include <cpu/pervasives.h>

#define LINE_SIZE (80*2)

static char* current;
char charMode = 0x0B;

static void scroll() {
  while(current >= (char*)FB_END) {
    word* src = (void*)FB_MEM+LINE_SIZE;
    word* dst = (void*)FB_MEM;
    while(src < (word*)FB_END) {
      *dst = *src; 
      *src = (charMode<<8) + ' ';
      dst++; src++;
    }
    current -= LINE_SIZE;
  }
}

void putChar(char c) {
  switch(c) {
  case '\n': {
    char* end = current + LINE_SIZE;
    end -= (end-(char*)FB_MEM) % LINE_SIZE;
    for(;current<end;current+=2) {
      *current = ' ';
      *(current+1) = charMode;
    }
    break;
  }
  case '\b':
    current-=2;
    *current = ' ';
    break;
  case '\0':
    break;
  default:
    *current=c;
    *(current+1) = charMode;
    current+=2;
    break;
  }
  scroll();
}

void printInt(int n) {
  char buf[32] = "";
  int i = 0;

  do { 
    buf[i] = n%10+'0';
    n/=10;
    i++;
  } while(n!=0);
  do {
    i--;
    putChar(buf[i]);
  } while(i>0);
}
void printHex(unsigned int n) {
  static char hdigs[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

  char buf[10] = "";
  int i = 0;

  do { 
    buf[i] = hdigs[n%16];
    n/=16;
    i++;
  } while(n!=0);
  do {
    i--;
    putChar(buf[i]);
  } while(i>0);
}
void printStr(char* s) {
  while(*s != '\0') {
    putChar(*s); s++;
  }
}

void printf(char* f,...) {
  int* args = (int*)&f;
  args++;
    
  while(*f!='\0') {
    switch(*f) {
    case '%':
      f++;
      switch(*f) {
      case 'd':
	printInt(*args);
	break;
      case 'c':
	putChar(*(char*)args);
	break;
      case 's':
	printStr((char*)*args);
	break;
      case 'x':
	printHex(*args);
	break;
      }
      args++;
      break;
    default:
      putChar(*f);
      break;
    }
    f++;
  }
  setCursor();
}

void clearFB() {
  for(current=(char*)FB_MEM;current<(char*)FB_END;current+=2) {
    *current = ' ';
    *(current+1) = charMode;
  }
  current=(char*)FB_MEM;
}

void setCursor() {
  dword temp = (current - (char*)FB_MEM) / 2;

  /* This sends a command to indicies 14 and 15 in the
   *  CRT Control Register of the VGA controller. These
   *  are the high and low bytes of the index that show
   *  where the hardware cursor is to be 'blinking'. To
   *  learn more, you should look up some VGA specific
   *  programming documents. A great start to graphics:
   *  http://www.brackeen.com/home/vga */
  outportb(0x3D4, 14);
  outportb(0x3D5, temp >> 8);
  outportb(0x3D4, 15);
  outportb(0x3D5, temp);
}
