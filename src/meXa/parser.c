#include <meXa/parser.h>
#include <util/array.h>
#include <meXa/dictionary.h>
#include <device/framebuffer.h>
#include <meXa/gear.h>

#define INRANGE(a,b,x) ((a) <= (x) && (x) <= (b))

CharClass classes[128] = {
  ['(']=OPAREN, [')']=CPAREN, ['[']=OPAREN, [']']=CPAREN,
  [' ']=SPACE, ['\t']=SPACE,
  ['\0']=END,
  ['0']=DIGIT, ['1']=DIGIT, ['2']=DIGIT, ['3']=DIGIT, ['4']=DIGIT,
  ['5']=DIGIT, ['6']=DIGIT, ['7']=DIGIT, ['8']=DIGIT, ['9']=DIGIT,
  ['"']=QUOTE, ['\'']=QUOTE,
  ['.']=OPERATOR, [':']=OPERATOR
};

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
    Gear* n = lookup(&rootGear,start);
    CUR = old;
    return n;
  }
  else
    return NULL;
}
Gear* atom(PState* pstate) {
  switch(classes[CUR]) {
  case OPAREN: { 
    char start = CUR;
    Torque* gears = newArray(sizeof(Torque) + sizeof(Array) + 16*sizeof(Gear*));
    gears->unit = ARRAY;
    Array* arr = AFTER(gears);
    arr->size = 0;
    Gear* ret = pure(gears);
    
    int n = 0;
    FORWARD; FREE;
    pstate->depth++;
    do {
      Gear* sub = EXPR; FREE;
      if(sub == NULL)
	break;
      arr->data[n] = cog(ret,sub);
      n++;
    } while(CUR != ']');
    arr->size = n;
    FORWARD;
    pstate->depth--;
        
    return (start=='[' ? ret : transmit(ret));
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
      Cog* la = cog(ret,a), *lb = cog(ret,b), *lo = cog(ret,o);
    
      ret->state = TORQUE;
      ret->torque = array(3,lo,la,lb);

      a = transmit(ret);
      break;
    }    
    case '.': {
      FORWARD;
      FREE;
      Gear* f = ATOM;
      if(f == NULL) return NULL;
    
      Gear* ret = newGear();
      Cog* la = cog(ret,a), *lf = cog(ret,f);
    
      ret->state = TORQUE;
      ret->torque = array(2,lf,la);
    
      a = transmit(ret);
      break;
    }
    default:  return a;
    }
  }
}
