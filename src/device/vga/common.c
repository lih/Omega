#include <device/vga/common.h>

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
  switch(r->addr) {
  case ACR_ADDR:
    inportb(INPUT_STATUS_1);
    outportb(ACR_ADDR,r->index);
    outportb(ACR_ADDR,b);
    break;
  case MISC_ADDR:
    outportb(MISC_DATA,b);
    break;
  default:
    outportb(r->addr,r->index);
    outportb(r->data,b);
  }
  if(getVGAReg(r) != b) {
    printf("Unable to write register %x (ind. %x)\n",r->addr,r->index);
    while(1);
  }
}
