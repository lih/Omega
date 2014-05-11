#include <cpu/pervasives.h>
#include <cpu/syscall.h>
#include <device/framebuffer.h>
#include <device/keyboard.h>
#include <init/repl.h>
#include <init/repl/map.h>
#include <init/repl/parser.h>

int  strcmp(char*,char*);
void palette();

Thunk rootThunk = {
  .state = PURE,
  .depth = 0,
  .pureVal = NULL,
  .ring = {
    .cRight = &rootThunk.ring,
    .pRight = &rootThunk.ring,
    .cLeft = &rootThunk.ring,
    .pLeft = &rootThunk.ring,
  }
};

Map rootNode = EMPTY;

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
  MapNode* n = getNode(&rootNode,"foo");
  showMap(rootNode); putChar('\n');
  getNode(&rootNode,"bar");
  showMap(rootNode); putChar('\n');
  getNode(&rootNode,"a");
  showMap(rootNode); putChar('\n');
  getNode(&rootNode,"b");
  showMap(rootNode); putChar('\n');
  getNode(&rootNode,"bloobidoo");
  showMap(rootNode); putChar('\n');
  n = getNode(&rootNode,"foo");
  n->t = pure(number(18));
  showMap(rootNode); putChar('\n');

  while(1) {
    printf("> ");
    int n = readn(79,'\n',buf);
    (void)n;
    PState st = {
      buf, 0
    };
#define pstate (&st)
    FREE;
    Thunk* t = EXPR;
#undef pstate
    if(t != NULL) {
      force(t);
      showVal(t->pureVal); putChar('\n');
      freeThunk(t);
    }
    else 
      printf("Couldn't understand the expression '%s'\n",buf);

    runCmd(buf);
  }
}
