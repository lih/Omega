#include <init/repl/parser.h>
#include <util/array.h>
#include <init/repl/map.h>
#include <device/framebuffer.h>

#define INRANGE(a,b,x) ((a) <= (x) && (x) <= (b))

void spaces(PState* pstate) {
  while(CUR == ' ' || CUR == '\t' || CUR == '\0') FORWARD;
}
Thunk* expr(PState* pstate) {
  switch(CUR) {
  case '[': 
  case '(': {
    char start = CUR;
    Value* thunks = newArray(sizeof(Value) + sizeof(Array) + 16*sizeof(Thunk*));
    thunks->shape = ARRAY;
    Array* arr = AFTER(thunks);
    
    int n = 0;
    FORWARD; FREE;
    do {
      arr->data[n] = EXPR; FREE;
      if(arr->data[n] == NULL)
	break;
      n++;
    } while(CUR != ']');
    arr->size = n;
    FORWARD;
    
    Thunk* ret = pure(thunks);
    return (start=='[' ? ret : eval(ret));
  }
  case ']':
  case ')': {
    if(pstate->depth == 0) {
      FORWARD;
      return EXPR;
    }
    else
      return NULL;
  }
  case '\0':
    return NULL;
  default: {
    if(INRANGE('0','9',CUR)) {
      int ret = CUR - '0';
      FORWARD;
      while(INRANGE('0','9',CUR)) {
	ret = ret*10 + (CUR-'0');
	FORWARD;
      }
      return pure(number(ret));
    }
    else if(CUR == '"') {
      FORWARD;
      char *start = &CUR;
      while(CUR != '"' && CUR != '\0') {
	if(CUR == '\\')
	  FORWARD;
	FORWARD;
      }
      CUR = '\0';
      Thunk* ret = pure(string(start));
      FORWARD;
      return ret;
    }
    else {
      char* start = &CUR;
      while(CUR != '(' && CUR != ')' && CUR != '[' && CUR != ']' 
	    && CUR != ' ' && CUR != '\t' && CUR != '\0') 
	FORWARD;
      char old = CUR;
      CUR = '\0';
      MapNode* n = getNode(&rootNode,start);
      CUR = old;
      
      return n->t;
    }
  }
  }
}

