#include <meXa/parser.h>
#include <util/array.h>
#include <meXa/dictionary.h>
#include <device/framebuffer.h>
#include <meXa/gear.h>

#define REASONABLE_SIZE 16

CharClass classes[128] = {
  ['(']=OPAREN, [')']=CPAREN, ['[']=OPAREN, [']']=CPAREN,
  [' ']=SPACE, ['\t']=SPACE,
  ['\0']=END,
  ['0']=DIGIT, ['1']=DIGIT, ['2']=DIGIT, ['3']=DIGIT, ['4']=DIGIT,
  ['5']=DIGIT, ['6']=DIGIT, ['7']=DIGIT, ['8']=DIGIT, ['9']=DIGIT,
  ['"']=QUOTE,
  ['.']=OPERATOR, [':']=OPERATOR,
  ['{']=APAREN, ['|']=OPERATOR, ['}']=CPAREN
};

static Gear* environment = &rootGear;

void spaces(PState* pstate) {
  while(classes[CUR] == SPACE) FORWARD;
}
Gear* ident(PState* pstate) {
  char* start = &CUR;
  while(classes[CUR] == REGULAR || classes[CUR] == DIGIT)
    FORWARD;

  if(&CUR > start) {
    char old = CUR;
    CUR = '\0';
    Gear* n = lookup(environment,start);
    CUR = old;
    return n;
  }
  else
    return NULL;
}
void abstractNode(MapNode* g,void* env) {
  Gear* ext = lookup(env,g->key);
  Gear* repl = pure(nil());
  repl->torque = abstract(link(repl,ext));

  replace(g->cog->down,repl);
}
Gear* atom(PState* pstate) {
  switch(classes[CUR]) {
  case OPAREN: { 
    char start = CUR;
    Gear* gears[REASONABLE_SIZE];
    int n = 0;

    pstate->depth++;
    FORWARD; FREE;
    do {
      gears[n] = EXPR; FREE;
      if(gears[n] == NULL)
	break;
      n++;
    } while(CUR != ']');
    FORWARD;
    pstate->depth--;
        
    if(start=='(' && n==1) 
      return gears[0];
    else {
      Gear* ret = pure(nil());
      ret->torque = array(n,ret,gears);

      return (start=='[' ? ret : mesh(ret));
    }
  }
  case CPAREN: {
    if(pstate->depth == 0) {
      FORWARD;
      return EXPR;
    }
    else
      return NULL;
  }
  case APAREN: {
    Gear* oldEnv = environment, *newEnv = pure(dictionary());
    Gear *expansion = NULL, *args, *ret = NULL;
    Gear* vars[REASONABLE_SIZE];
    int nvars = 0;

    environment = newEnv;
    FORWARD;
    while((vars[nvars] = (FREE,IDENT))) nvars++;
    if(CUR == '|')
      expansion = (FORWARD,FREE,EXPR);
    environment = oldEnv;
    
    args = pure(nil());
    args->torque = array(nvars,args,vars);
    
    Map* m = AFTER(newEnv->torque);
    forNodes(*m,oldEnv,abstractNode);
    
    Array* va = AFTER(args->torque);
    DOTIMES(i,va->size) {
      Cog** c = AFTER(va->data[i]->down->torque);
      unlink(*c);
      *c = NULL;
    }
    ret = pure(func(instanciate));
    link(ret,expansion);
    link(ret,args);
    freeGear(newEnv);

    return ret;
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
    char old = CUR;
    CUR = '\0';
    Gear* ret = pure(string(start));
    CUR = old;
    if(old != '\0') FORWARD;
    return ret;
  }
  case REGULAR: return IDENT;
  default: return NULL;
  }
}
Gear* expr(PState* pstate) {
  Gear* a = ATOM;
  if(a == NULL) return NULL;
  
  while(1) {
    FREE;
    switch(CUR) {
    case ':': {
      FORWARD;
      FREE;
      Gear* o = ATOM;
      FREE;
      Gear* b = ATOM;
      if(o == NULL || b == NULL) return NULL;
      Gear* ret = newGear();
    
      ret->state = TORQUE;
      ret->torque = arrayn(3,ret,o,a,b);

      a = mesh(ret);
      break;
    }    
    case '.': {
      FORWARD;
      FREE;
      Gear* f = ATOM;
      if(f == NULL) return NULL;
    
      Gear* ret = newGear();
          
      ret->state = TORQUE;
      ret->torque = arrayn(2,ret,f,a);
    
      a = mesh(ret);
      break;
    }
    default:  return a;
    }
  }
}
