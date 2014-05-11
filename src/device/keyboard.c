#include <core/feature.h>
#include <cpu/descriptors.h>
#include <cpu/interrupt.h>
#include <cpu/pervasives.h>
#include <cpu/syscall.h>
#include <device/framebuffer.h>
#include <device/vga.h>

Selector keyboardGate;
Semaphore scanSem = 0;
byte currentScan;
Semaphore charSem = 0;
char currentChar;

void handleKey(byte);
extern void handleKeyboard();

extern int running;
char layout[128] = {
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
  0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
  '\'', '`',   0,		/* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
  0,	/* Alt */
  ' ',	/* Space bar */
  0,	/* Caps lock */
  0,	/* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,	/* < ... F10 */
  0,	/* 69 - Num lock*/
  0,	/* Scroll Lock */
  0,	/* Home key */
  0,	/* Up Arrow */
  0,	/* Page Up */
  '-',
  0,	/* Left Arrow */
  0,
  0,	/* Right Arrow */
  '+',
  0,	/* 79 - End key*/
  0,	/* Down Arrow */
  0,	/* Page Down */
  0,	/* Insert Key */
  0,	/* Delete Key */
  0,   0,   0,
  0,	/* F11 Key */
  0,	/* F12 Key */
  0,	/* All other keys are undefined */
};

static void initKeyboard() {
  require(&_universe_);

  TSS* keyTSS = (void*)KEY_STACK-sizeof(TSS);
  *keyTSS = tss(kernelSpace.pageDir,handleKeyboard,keyTSS);
  keyboardGate = addDesc(&gdt,tssDesc(keyTSS,0));
}
Feature _keyboard_ = {
  .state = DISABLED,
  .label = "keyboard",
  .initialize = &initKeyboard
};

byte ctrl = 0, shift = 0, alt = 0;
char currentChar;

void handleKeyboard() {
  while(1) {
    byte scan = inportb(0x60);

    currentScan = scan;
    releaseSem(&scanSem,0);
    scanSem = 0;
    if(!(scan & 0x80)) {
      switch(layout[scan]) {
      case 0:
	switch(scan) {
	case 42:			/* Shift */
	  shift = 1;
	  break;
	case 29:			/* Ctrl */
	  ctrl = 1;
	  break;
	case 56: 			/* Alt */
	  alt = 1;
	  break;
	default:
	  printf("Unassociated scancode %d\n",scan);
	}
	break;
      
      case 27: /* ESCAPE */
	if(ctrl && alt)
	  shutdown();
	break;
      
      default: {
	currentChar = layout[scan] - shift*32;
	putChar(currentChar);
	releaseSem(&charSem,0);
	charSem = 0;
	break;
      }
      }

      setCursor();
    }
    else switch(scan) {
      case 170:			/* Unshift */
	shift = 0;
	break;
      case 157: 			/* Unctrl */
	ctrl = 0;
	break;
      case 184:			/* Unalt */
	alt = 0;
	break;
      }

    outportb(0x20,0x20);
    asm __volatile ( "iret" );
  } 
}

char readChar() {
  syscall_acquire(&charSem);
  return currentChar;
}
byte readScan() {
  syscall_acquire(&scanSem);
  return currentScan;
}
int readn(int n, char limit, char* buf) {
  int i;
  for(i=0;i<n;i++) {
    buf[i] = readChar();
    if(buf[i] == '\b')
      i-=2;
    if(buf[i] == limit)
      break;
  }
  buf[i] = '\0';
  return i;
}

