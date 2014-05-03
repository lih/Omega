#include "feature.h"
#include "pervasives.h"
#include "interrupt.h"
#include "descriptors.h"
#include "vga.h"

Selector keyboardGate;

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

  TSS* keyTSS = KEY_STACK-sizeof(TSS);
  *keyTSS = tss(kernelSpace.pageDir,handleKeyboard,KEY_STACK-sizeof(TSS));
  keyboardGate = addDesc(&gdt,tssDesc(keyTSS,0));
}
Feature _keyboard_ = {
  .state = DISABLED,
  .label = "keyboard",
  .initialize = &initKeyboard
};

byte ctrl = 0, shift = 0, alt = 0;

void handleKey(byte scan) {
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
      if(ctrl)
	switchMode();
      break;
      
    default:
      putChar(layout[scan] - (shift*32));
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
} 

