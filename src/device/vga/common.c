#include <device/vga/common.h>
#include <x86/pervasives.h>

byte getVGAReg(VGAReg* r) {
  switch(r->addr) {
  case ACR_ADDR:
    inportb(INPUT_STATUS_1);
    outportb(ACR_ADDR,r->index);
    return inportb(ACR_DATA);
  case MISC_ADDR:
    return inportb(MISC_ADDR);
  default:
    outportb(r->addr,r->index);
    return inportb(r->data);
  }
}
void setVGAReg(VGAReg* r,byte b) {
  byte newVal;
  do {
    switch(r->addr) {
    case ACR_ADDR:
      inportb(INPUT_STATUS_1);
      outportb(ACR_ADDR,r->index);
      outportb(ACR_ADDR,b);
      outportb(ACR_ADDR,r->index);
      newVal = inportb(ACR_ADDR);
      break;
    case MISC_ADDR:
      outportb(MISC_DATA,b);
      newVal = inportb(MISC_ADDR);
      break;
    default:
      outportb(r->addr,r->index);
      outportb(r->data,b);
      newVal = inportb(r->data);
    }
  } while(newVal != b);
}
