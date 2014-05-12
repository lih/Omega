#include <core/feature.h>
#include <cpu/descriptors.h>
#include <cpu/interrupt.h>
#include <cpu/pervasives.h>
#include <core/syscall.h>
#include <device/framebuffer.h>
#include <device/vga.h>
#include <device/keyboard.h>

Selector keyboardGate;
Semaphore scanSem = 0;
byte currentScan;
Semaphore charSem = 0;
char currentChar;

void handleKey(byte);
extern void handleKeyboard();

Layout qwerty = {
  [SC_1]={'1','!'}, [SC_2]={'2','@'}, [SC_3]={'3','#'}, [SC_4]={'4','$'}, [SC_5]={'5','%'},
  [SC_6]={'6','^'}, [SC_7]={'7','&'}, [SC_8]={'8','*'}, [SC_9]={'9','('}, [SC_0]={'0',')'},

  [SC_A]={'a','A'},[SC_B]={'b','B'},[SC_C]={'c','C'},[SC_D]={'d','D'},[SC_E]={'e','E'},[SC_F]={'f','F'},
  [SC_G]={'g','G'},[SC_H]={'h','H'},[SC_I]={'i','I'},[SC_J]={'j','J'},[SC_K]={'k','K'},[SC_L]={'l','L'},
  [SC_M]={'m','M'},[SC_N]={'n','N'},[SC_O]={'o','O'},[SC_P]={'p','P'},[SC_Q]={'q','Q'},[SC_R]={'r','R'},
  [SC_S]={'s','S'},[SC_T]={'t','T'},[SC_U]={'u','U'},[SC_V]={'v','V'},[SC_W]={'w','W'},[SC_X]={'x','X'},
  [SC_Y]={'y','Y'},[SC_Z]={'z','Z'},

  [SC_TAB]={'\t'}, [SC_BACKSPACE]={'\b'},

  [SC_MINUS]={'-','_'}, 
  [SC_EQUAL]={'=','+'}, 
  [SC_SEMICOLON]={';',':'},
  [SC_QUOTE]={'\'','"'},
  [SC_BACKQUOTE]={'`','~'},
  [SC_BACKSLASH]={'\\','|'},
  [SC_COMMA]={',','<'},
  [SC_DOT]={'.','>'},
  [SC_SLASH]={'/','?'},
  [SC_LEFTBRK]={'[','{'}, [SC_RIGHTBRK]={']','}'},
  [SC_SPACE]={' '}, [SC_ENTER]={'\n'}
};
Layout azerty = {
  [SC_1]={'&','1'}, [SC_2]={0,'2'}, [SC_3]={'"','3'}, [SC_4]={'\'','4'}, [SC_5]={'(','5'},
  [SC_6]={'-','6'}, [SC_7]={0,'7'}, [SC_8]={'_','8'}, [SC_9]={0,'9'}, [SC_0]={0,'0'},

  [SC_Q]={'a','A'},[SC_B]={'b','B'},[SC_C]={'c','C'},[SC_D]={'d','D'},[SC_E]={'e','E'},[SC_F]={'f','F'},
  [SC_G]={'g','G'},[SC_H]={'h','H'},[SC_I]={'i','I'},[SC_J]={'j','J'},[SC_K]={'k','K'},[SC_L]={'l','L'},
  [SC_SEMICOLON]={'m','M'},[SC_N]={'n','N'},[SC_O]={'o','O'},[SC_P]={'p','P'},[SC_A]={'q','Q'},[SC_R]={'r','R'},
  [SC_S]={'s','S'},[SC_T]={'t','T'},[SC_U]={'u','U'},[SC_V]={'v','V'},[SC_Z]={'w','W'},[SC_X]={'x','X'},
  [SC_Y]={'y','Y'},[SC_W]={'z','Z'},


  [SC_TAB]={'\t'}, [SC_BACKSPACE]={'\b'},

  [SC_MINUS]={')'}, 
  [SC_EQUAL]={'=','+'}, 
  [SC_M]={',','?'},
  [SC_COMMA]={';','.'},
  [SC_QUOTE]={0,'%'},
  [SC_BACKSLASH]={'*'},
  [SC_DOT]={':','/'},
  [SC_SLASH]={'!'},
  [SC_LEFTBRK]={'^'}, [SC_RIGHTBRK]={'$'},
  [SC_SPACE]={' '}, [SC_ENTER]={'\n'}
};
Layout *currentLayout = &qwerty;

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
      switch(scan) {
      case SC_LSHIFT:			/* Shift */
	shift = 1;
	break;
      case SC_LCTRL:			/* Ctrl */
	ctrl = 1;
	break;
      case SC_ALT: 			/* Alt */
	alt = 1;
	break;
      case SC_ESCAPE: /* ESCAPE */
	if(ctrl && alt)
	  shutdown();
	break;
      default: 
	if((*currentLayout)[scan][shift] != 0) {
	  currentChar = (*currentLayout)[scan][shift];
	  releaseSem(&charSem,0);
	  charSem = 0;
	  break;
	}
      }

      setCursor();
    }
    else switch(scan^0x80) {
      case SC_LSHIFT:			/* Unshift */
	shift = 0;
	break;
      case SC_LCTRL: 			/* Unctrl */
	ctrl = 0;
	break;
      case SC_ALT:			/* Unalt */
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
    if(buf[i] == '\b') {
      if(i>=1) {
	putChar('\b');
	i-=2;
      }
      else i--;
    }
    else {
      putChar(buf[i]);
      if(buf[i] == limit)
	break;
    }
    setCursor();
  }
  buf[i] = '\0';
  return i;
}

