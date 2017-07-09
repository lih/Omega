#ifndef INCLUDED_KEYBOARD
#define INCLUDED_KEYBOARD

#include <util/feature.h>
#include <x86/descriptors.h>
#include <core/life.h>

extern Feature _keyboard_;

typedef enum {
  SC_ESCAPE=1, SC_F1=59, SC_F2, SC_F3, SC_F4, SC_F5, SC_F6, SC_F7, SC_F8, SC_F9, SC_F10, SC_F11=86, SC_F12,
  SC_BACKQUOTE=41, SC_1=2, SC_2, SC_3, SC_4, SC_5, SC_6, SC_7, SC_8, SC_9, SC_0, SC_MINUS, SC_EQUAL, SC_BACKSPACE,
  SC_TAB, SC_Q, SC_W,SC_E, SC_R, SC_T, SC_Y, SC_U, SC_I, SC_O, SC_P, SC_LEFTBRK, SC_RIGHTBRK, SC_ENTER,
  SC_CAPSLOCK=58, SC_A=30, SC_S, SC_D, SC_F, SC_G, SC_H, SC_J, SC_K, SC_L, SC_SEMICOLON, SC_QUOTE, 
  SC_LSHIFT=42, SC_BACKSLASH, SC_Z, SC_X, SC_C, SC_V, SC_B, SC_N, SC_M, SC_COMMA, SC_DOT, SC_SLASH, SC_RSHIFT,

  SC_LCTRL=29, SC_META=55, SC_ALT, SC_SPACE,

  SC_NUMLOCK=69, SC_SCROLLLOCK, SC_HOME, SC_PGUP=73, SC_END,
  SC_PAGEDOWN=80, SC_INSERT, SC_DELETE,

  SC_KPLUS=78, SC_KMINUS=74,
  SC_UP=72, SC_LEFT=75, SC_DOWN=80, SC_RIGHT=77,
} ScanCode;
typedef char Layout[128][2];

extern Layout azerty,qwerty;
extern Layout* currentLayout;

extern Selector keyboardGate;

extern Semaphore scanSem;
extern byte currentScan;
extern Semaphore charSem;
extern char currentChar;

char readChar();
byte readScan();
int readn(int n, char limit, char* buf);

#endif
