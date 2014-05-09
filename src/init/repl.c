#include <init/repl.h>
#include <device/keyboard.h>
#include <device/framebuffer.h>

int strcmp(char* s1,char* s2) {
  while(*s1 == *s2 && *s1 != '\0')
    { s1++; s2++; }
  
  return *s2-*s1;
}
void runCmd(char* buf) {
  if(strcmp(buf,"hello") == 0)
    printf("Hello to you too !\n");
  else if(strcmp(buf,"quit") == 0) {
    printf("Shutting down in 2 seconds...\n");
    syscall_wait(2,0);
    shutdown();
  }
  else if(strcmp(buf,"graph") == 0) {
    syscall_bios(BIOS_MODE13H);
    palette();
  }
  else if(strcmp(buf,"cli") == 0)
    syscall_bios(BIOS_MODE03H);
}
void repl() {
  char buf[80];
  while(1) {
    printf("> ");
    int n = readn(79,'\n',buf);

    runCmd(buf);
  }
}
