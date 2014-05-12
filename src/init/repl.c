#include <cpu/pervasives.h>
#include <core/syscall.h>
#include <device/framebuffer.h>
#include <device/keyboard.h>
#include <init/repl.h>
#include <init/repl/map.h>
#include <init/repl/parser.h>
#include <util/array.h>

int  strcmp(char*,char*);
void palette();

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

#define IDENT ident(pstate)
char* ident(PState* pstate) {
  char* start = &CUR;
  while(CUR != '\0' && CUR != ' ' && CUR != '\t' && CUR != '=') FORWARD;
  char* end = &CUR;
  FREE;
  if(CUR=='=') {
    FORWARD;
    int l = end - start;
    char* ret = newArray(l+1);
    memcpy(ret,start,l);
    ret[l] = '\0';
    return ret;
  }
  else {
    pstate->str = start;
    return NULL;
  }
}

Value* array_at(Array* arr) {
  if(arr->size == 3) {
    Value* varr = force(arr->data[1]);
    Value* vn = force(arr->data[2]);
    if(varr->shape == ARRAY && vn->shape == NUMBER) {
      int* n = AFTER(vn);
      Array* arr = AFTER(varr);
      return force(arr->data[*n]);
    }
    else
      printf("Wrong argument types for array_at\n");
  }
  else
    printf("Wrong number of arguments for function array_at\n");
  
  return nil();
}

void repl() {
  char buf[80];

  define("@",pure(func(array_at)));

  while(1) {
    printf("> ");
    int n = readn(79,'\n',buf);
    (void)n;
    PState st = {
      buf, 0
    };
#define pstate (&st)
    FREE;
    char* var = IDENT;
    FREE;
    Thunk* t = EXPR;
#undef pstate
    if(var==NULL) var = "it";
    if(t != NULL) {
      showVal(force(t)); putChar('\n');
      printf("%s: %x\n",var,t);
      define(var,t);
    }
    else 
      printf("Couldn't understand the expression '%s'\n",buf);

    runCmd(buf);
  }
}
