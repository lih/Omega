#include <init/repl.h>
#include <device/keyboard.h>
#include <device/framebuffer.h>

void memcpy(byte* dst_,byte const *src_,int n) {
  dword *src = src_,*dst = dst_;
  dword* end = src+(n>>2);
  
  while(src != end) {
    *dst = *src; src++; dst++;
  }
  int i;
  for(i=0;i<n&3;i++)
    ((byte*)end)[i] = ((byte*)src)[i];
}
int strcmp(char* s1,char* s2) {
  while(*s1 == *s2 && *s1 != '\0')
    { s1++; s2++; }
  
  return *s2-*s1;
}
int strlen(char* s) {
  char* c = s;
  while(*c != '\0') c++;
  return c-s;
}

Pool linkPool = { 0, sizeof(Link) };
Pool thunkPool = { 0, sizeof(Thunk) };

#define FORRING(i,r,t) for(i=(r).right##t ; i != &(r);i=i->right##t )

void nothing(struct Thunk* _) { }
Thunk* newThunk() {
  Thunk* ret = poolAlloc(sizeof(Thunk));
  ret->depth = 10000000;
  Link* ring = &ret->parentRing;
  ring->rightUp = ring; 
  ring->rightDown = ring; 
  ring->leftUp = ring;
  ring->leftDown = ring; 
  ring = &ret->childRing;
  ring->rightUp = ring; 
  ring->rightDown = ring; 
  ring->leftUp = ring;
  ring->leftDown = ring; 
  ret->pureVal = NULL;
  ret->initializer = nothing;
}
Value* number(int n) {
  Value* ret = newArray(sizeof(Value) + sizeof(int));
  ret->shape = NUMBER;

  int* v = &ret->data;
  *v = n;
  return ret;
}
Value* string(char* s) {
  int l = strlen(s);
  Value* ret = newArray(sizeof(Value) + sizeof(String) + l + 1);
  ret->shape = STRING;
  
  String* str = &ret->data;
  str->sz = l;
  memcpy(&str->data,s,l+1);
  
  return ret;
}
Value* array(int n,...) {
  Thunk** args = &n + 1;
  Value* ret = newArray(sizeof(Value) + sizeof(Array) + n*sizeof(Thunk*));
  ret->shape = ARRAY;

  Array* arr = &ret->data;
  int i;
  arr->size = n;
  for(i=0;i<n;i++)
    arr->data[i] = args[i];

  return ret;
}
Value* func(Function f) {
  Value* ret = newArray(sizeof(Value) + sizeof(Function));
  ret->shape = FUNCTION;
  Function* f2 = &ret->data;
  *f2 = f;
  return ret;
}

void link(Thunk* f,Thunk* s) {
  Link* l = poolAlloc(&linkPool);
  l->down = s;
  l->rightDown = s->parentRing.rightDown;
  l->leftDown = &s->parentRing;
  l->leftDown->rightDown = l;
  l->rightDown->leftDown = l;
  
  l->up = f; 
  l->rightUp = s->childRing.rightUp;
  l->leftUp = &s->childRing;
  l->leftUp->rightUp = l;
  l->rightUp->leftUp = l;
}
Thunk* pure(Value* v) {
  Thunk* ret = newThunk();
  ret->state = PURE;
  ret->pureVal = v;
  if(v->shape == ARRAY) {
    int i;
    Array* arr = &v->data;
    for(i=0;i<arr->size;i++)
      link(ret,arr->data[i]);
  }
  return ret;
}
void evaluate(Thunk* t) {
  Thunk* child = t->childRing.rightUp->down;
  force(child);
  switch(child->pureVal->shape) {
  case ARRAY: {
    Array* arr = &child->pureVal->data;
    Array* newArr = newArray(sizeof(Array) + arr->size*sizeof(arr->data[0]));
    
    int i;
    newArr->size = arr->size;
    for(i=0;i<arr->size;i++)
      newArr->data[i] = eval(arr->data[i]);
    
    Thunk* ft = newArr->data[0];
    force(ft);
    switch(ft->pureVal->shape) {
    case FUNCTION: {
      Function* f = &ft->pureVal->data;
           
      Thunk* res = (*f)(newArr);
      force(res);
      t->pureVal = res->pureVal;
      res->pureVal = NULL;
      freeThunk(res);
      break;
    }
    }

    freeArray(newArr);
    break;
  }
  default:
    t->pureVal = child->pureVal;
    break;
  }
}
Thunk* eval(Thunk* t) {
  Thunk* ret = newThunk();
  ret->state = THUNK;
  ret->initializer = evaluate;
  link(ret,t);
  return ret;
}

void freeThunk(Thunk* t) {
  if(t->parentRing.rightDown == &t->parentRing) {
    Link *child;
    FORRING(child,t->childRing,Up) {
      child->rightDown->leftDown = child->leftDown;
      child->leftDown->rightDown = child->rightDown;
      freeThunk(child->down);
    }
    while(t->childRing.rightUp != &t->childRing) {
      child = t->childRing.rightUp;
      t->childRing.rightUp = child->rightUp;
      poolFree(&linkPool,child);
    }
    if(t->pureVal != NULL) freeArray(t->pureVal);
    poolFree(&thunkPool,t);
  }
}

typedef struct {
  char* str;
  int depth;
} PState;

#define FREE spaces(pstate)
#define EXPR expr(pstate)
#define CUR (*(pstate->str))
#define FORWARD ((pstate->str)++)
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
    Array* arr = &thunks->data;
    
    int n = 0;
    FREE;
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
  default:
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
      int l = &CUR - start;
      CUR = '\0';
      Thunk* ret = pure(string(start));
      FORWARD;
      return ret;
    }
    else {
      char* start = &CUR;
      while(CUR != '(' && CUR != ')' && CUR != '[' && CUR != ']' 
	    && CUR != ' ' && CUR != '\t' && CUR == '\0')
	FORWARD;
      char old = CUR;
      CUR = '\0';
      Value* v = string(start);
      CUR = old;
      return pure(v);
    }
  }

}

void force(Thunk* t) {
  switch(t->state) {
  case PURE:
    break;
  case THUNK: {
    t->state = LOCKED;
    t->initializer(t);
    t->state = PURE;
    break;
  }
  }
}
void invalidate(Thunk* t) {
  Link* parent;
  FORRING(parent,t->parentRing,Down) 
    if(parent->up->state == PURE) {
      parent->up->state = THUNK;
      invalidate(parent->up);
    }
}
int parentDepth(Thunk* t) {
  Link* l;
  int depth = 10000000;
  FORRING(l,t->parentRing,Down) 
    if(l->up->depth < depth)
      depth = l->up->depth;
  return depth;
}
void rebase(Thunk* t,int depth) {
  Link *child,*parent;
  t->depth = depth;
  
  FORRING(child,t->childRing,Up) 
    if(child->down->depth > depth)
      rebase(child->down,depth+1);
}
void replace(Thunk* old,Thunk* new) {
  invalidate(old);
  
  Link* parent;
  FORRING(parent,old->parentRing,Down) {
    parent->leftDown->rightDown = parent->rightDown;
    parent->rightDown->leftDown = parent->leftDown;
    parent->rightDown = new->parentRing.rightDown;
    parent->leftDown = &new->parentRing;
    parent->leftDown->rightDown = parent;
    parent->rightDown->leftDown = parent;
    parent->down = new;
  }

  freeThunk(old);
  rebase(new,parentDepth(new)+1);
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
