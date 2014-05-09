#ifndef INCLUDED_KEYBOARD
#define INCLUDED_KEYBOARD

#include <core/feature.h>
#include <cpu/descriptors.h>
#include <core/schedule.h>

extern Feature _keyboard_;

extern char layout[128];
extern Selector keyboardGate;

extern Semaphore scanSem;
extern byte currentScan;
extern Semaphore charSem;
extern byte currentChar;

char readChar();
byte readScan();
int readn(int n, char limit, char* buf);

#endif
