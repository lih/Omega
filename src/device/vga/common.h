#ifndef INCLUDED_VGA_COMMON
#define INCLUDED_VGA_COMMON

#include <cpu/memory.h>

typedef struct {
  word addr,index,data;
} VGAReg;
typedef struct {
  byte plane0:1;
  byte plane1:1;
  byte plane2:1;
  byte plane3:1;
} PACKED SetReset;
typedef struct {
  byte ioas:1;
  byte ramEnable:1;
  byte clockSelect:2;
  byte _:1;
  byte oepage:1;
  byte hsyncp:1;
  byte vsyncp:1;
} PACKED MiscOutput;
#define REG(t,i) { t##_ADDR , i , t##_DATA }
#define SETREG(x,r) setVGAReg(&(r),AS(byte,(x)->r))
#define GETREG(x,r) AS(byte,(x)->r) = getVGAReg(&(r))
#define MODREG(r,t,x) { t REG; AS(byte,REG) = getVGAReg(&(r)); x; setVGAReg(&(r),AS(byte,REG)); } 

#define VGA_ADDR 0x3ce
#define VGA_DATA 0x3cf
#define SEQ_ADDR 0x3c4
#define SEQ_DATA 0x3c5
#define ACR_ADDR 0x3c0
#define ACR_DATA 0x3c1
#define MISC_ADDR 0x3cc
#define MISC_DATA 0x3c2
#define COLOR_DATA 0x3c8
#define COLOR_ADDR 0x3c9
#define INPUT_STATUS_1 0x3da
#define INPUT_STATUS_2 0x3c2

void setVGAReg(VGAReg* r,byte b);
byte getVGAReg(VGAReg* r);

#endif
