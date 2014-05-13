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

int identEq(PState* pstate) {
  char* start = &CUR;
  int ret = 0;
  while(CUR != '=' && (classes[CUR] == REGULAR || classes[CUR] == DIGIT))
    FORWARD;
 
  char* end = &CUR;
  if(start != end) {
    FREE; if(CUR != '=') {
      pstate->str = start;
      return 0;
    }
    FORWARD;
    char old = *end;
    *end='\0';
    ret = strlen(start);
    *end=old;
  }
  return ret;
}
#undef IDENT
#define IDENT identEq(pstate)

Value* disowned(Value* v) {
  v->owned = 0;
  return v;
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
    else if(varr->shape == DICTIONARY && vn->shape == STRING) {
      String* s = AFTER(vn);
      return force(lookup(arr->data[1]->down,s->data));      
    }
    else printf("Wrong argument types for array_at\n");
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
Value* builtin_plus(Array* args) {
  int ret = 0;
  int i;
  for(i=1;i<args->size;i++) {
    Value* v = args->data[i]->down->pureVal;
    if(v->shape == NUMBER)
      ret += *(int*)AFTER(v);
  }
  return disowned(number(ret));
}
#define VALUE_AT(a,i) force((a)->data[i]->down)
Value* builtin_eq(Array* args) {
  if(args->size == 3) {
    Value *a = VALUE_AT(args,1), *b = VALUE_AT(args,2);
    if(a->shape == NUMBER && b->shape == NUMBER) {
      int xa = *(int*)AFTER(a), xb = *(int*)AFTER(b);
      return (xa==xb ? number(0) : nil());
    } 
  }
  return nil();
}

void repl() {
  char buf[80];

  define("@",pure(func(array_at)));
  define("if",pure(func(builtin_if)));
  define("+",pure(func(builtin_plus)));
  define("=",pure(func(builtin_eq)));

  while(1) {
    printf("> ");
    int n = readn(79,'\n',buf);
    (void)n;
    PState st = {
      buf, 0
    };
#define pstate (&st)
    FREE;
    char* vstart = &CUR;
    int vsize = IDENT;
    FREE;
    Thunk* t = EXPR;
    if(t != NULL) {
      showVal(force(t)); putChar('\n');
      if(vsize > 0) {
	char old = vstart[vsize];
	vstart[vsize] = '\0';
	define(vstart,t);
	vstart[vsize] = old;
      }
      else
	freeThunk(t);
      
      /* printf("Defined '%s' at %x\n",var,t); */
    }
    else 
      printf("Couldn't understand the expression '%s'\n",buf);

    runCmd(buf);
#undef pstate
  }
}

