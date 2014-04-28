#ifndef FRAMEBUFFER
#define FRAMEBUFFER

extern char charMode;

void putChar(char);
void printInt(int);
void printStr(char*);
void printf(char*,...);
void clearFB();
void setCursor();

#endif
