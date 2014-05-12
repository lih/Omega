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
    Value* varr = force(arr->data[1]->down);
    Value* vn = force(arr->data[2]->down);
    if(varr->shape == ARRAY && vn->shape == NUMBER) {
      int* n = AFTER(vn);
      Array* arr = AFTER(varr);
      return force(arr->data[*n]->down);
    }
    else
      printf("Wrong argument types for array_at\n");
  }
  else
    printf("Wrong number of arguments for function array_at\n");
  
  return nil();
}
Value* builtin_if(Array* args) {
  if(args->size == 4) {
    Value* cond = force(args->data[1]->down);
    if(cond->shape == NIL)
      return force(args->data[3]->down);
    else
      return force(args->data[2]->down);
  }
  else
    printf("Wrong number of arguments to 'if' function\n");
  return nil();
}

void repl() {
  char buf[80];

  define("@",pure(func(array_at)));
  define("if",pure(func(builtin_if)));

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
    if(t != NULL) {
      showVal(force(t)); putChar('\n');
      if(var != NULL) {
	define(var,t);
	/* printf("Defined '%s' at %x\n",var,t); */
      }
      else 
	freeThunk(t);
    }
    else 
      printf("Couldn't understand the expression '%s'\n",buf);

    runCmd(buf);
  }
}
