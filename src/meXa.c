#include <x86/pervasives.h>
#include <core/syscall.h>
#include <device/framebuffer.h>
#include <device/keyboard.h>
#include <meXa.h>
#include <meXa/dictionary.h>
#include <meXa/parser.h>
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

Gear* array_at(Array* arr) {
  if(arr->size == 3) {
    Torque* varr = torque(arr->data[1]->down);
    Torque* vn = torque(arr->data[2]->down);
    if(varr->unit == ARRAY && vn->unit == NUMBER) {
      int* n = AFTER(vn);
      Array* arr = AFTER(varr);
      return arr->data[*n]->down;
    }
    else if(varr->unit == DICTIONARY && vn->unit == STRING) {
      String* s = AFTER(vn);
      return lookup(arr->data[1]->down,s->data);      
    }
    else printf("Wrong argument types for array_at\n");
  }
  else
    printf("Wrong number of arguments for function array_at\n");
  
  return pure(nil());
}
Gear* builtin_if(Array* args) {
  if(args->size == 4) {
    Torque* cond = torque(args->data[1]->down);
    if(cond->unit == NIL)
      return args->data[3]->down;
    else
      return args->data[2]->down;
  }
  else
    printf("Wrong number of arguments to 'if' function\n");
  return pure(nil());
}
Gear* builtin_plus(Array* args) {
  int ret = 0;
  int i;
  for(i=1;i<args->size;i++) {
    Torque* v = torque(args->data[i]->down);
    if(v->unit == NUMBER)
      ret += *(int*)AFTER(v);
  }
  return pure(number(ret));
}
#define VALUE_AT(a,i) torque((a)->data[i]->down)
Gear* builtin_eq(Array* args) {
  if(args->size == 3) {
    Torque *a = VALUE_AT(args,1), *b = VALUE_AT(args,2);
    if(a->unit == NUMBER && b->unit == NUMBER) {
      int xa = *(int*)AFTER(a), xb = *(int*)AFTER(b);
      return pure(xa==xb ? number(0) : nil());
    } 
  }
  return pure(nil());
}

void repl() {
  char buf[80];

  define("@",pure(func(array_at)));
  define("if",pure(func(builtin_if)));
  define("+",pure(func(builtin_plus)));
  define("=",pure(func(builtin_eq)));

  while(1) {
    printf("| ");
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
    Gear* t = EXPR;
    if(t != NULL) {
      Torque* v = torque(t);
      printStr("= "); showTorque(v); putChar('\n');
      if(vsize > 0) {
	char old = vstart[vsize];
	vstart[vsize] = '\0';
	define(vstart,t);
	vstart[vsize] = old;
      }
      else
	freeGear(t);
      
      /* printf("Defined '%s' at %x\n",var,t); */
    }
    else 
      printf("Couldn't understand the expression '%s'\n",buf);

    runCmd(buf);
#undef pstate
  }
}

