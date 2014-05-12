#include <init/repl/parser.h>
#include <util/array.h>
#include <init/repl/map.h>
#include <device/framebuffer.h>

#define INRANGE(a,b,x) ((a) <= (x) && (x) <= (b))

CharClass classes[128] = {
  ['(']=OPAREN, [')']=CPAREN, ['[']=OPAREN, [']']=CPAREN,
  [' ']=SPACE, ['\t']=SPACE,
  ['\0']=END,
  ['0']=DIGIT, ['1']=DIGIT, ['2']=DIGIT, ['3']=DIGIT, ['4']=DIGIT,
  ['5']=DIGIT, ['6']=DIGIT, ['7']=DIGIT, ['8']=DIGIT, ['9']=DIGIT,
  ['"']=QUOTE, ['\'']=QUOTE
};

void spaces(PState* pstate) {
  while(classes[CUR] == SPACE) FORWARD;
}
Thunk* ident(PState* pstate) {
  char* start = &CUR;
  while(classes[CUR] == REGULAR || classes[CUR] == DIGIT)
    FORWARD;

  if(&CUR > start) {
    char old = CUR;
    CUR = '\0';
    Thunk* n = lookup(&rootThunk,start);
    CUR = old;
    return n;
  }
  else
    return pure(nil());
}
Thunk* expr(PState* pstate) {
  switch(classes[CUR]) {
  case OPAREN: { 
    char start = CUR;
    Value* thunks = newArray(sizeof(Value) + sizeof(Array) + 16*sizeof(Thunk*));
    thunks->shape = ARRAY;
    Array* arr = AFTER(thunks);
    arr->size = 0;
    Thunk* ret = pure(thunks);
    
    int n = 0;
    FORWARD; FREE;
    pstate->depth++;
    do {
      Thunk* sub = EXPR; FREE;
      if(sub == NULL)
	break;
      arr->data[n] = link(ret,sub);
      n++;
    } while(CUR != ']');
    arr->size = n;
    FORWARD;
    pstate->depth--;
        
    return (start=='[' ? ret : eval(ret));
  }
  case CPAREN: {
    if(pstate->depth == 0) {
      FORWARD;
      return EXPR;
    }
    else
      return NULL;
  }
  case DIGIT: {
    int ret = CUR - '0';
    FORWARD;
    while(classes[CUR]==DIGIT) {
      ret = ret*10 + (CUR-'0');
      FORWARD;
    }
    return pure(number(ret));
  }
  case QUOTE: {
    FORWARD;
    char *start = &CUR;
    while(!(classes[CUR] == QUOTE || classes[CUR] == END)) {
      if(CUR == '\\') FORWARD;
      FORWARD;
    }
    CUR = '\0';
    Thunk* ret = pure(string(start));
    FORWARD;
    return ret;
  }
  case REGULAR: return IDENT;
  default: return NULL;
  }
}

