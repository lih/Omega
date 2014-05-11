#include <init/repl/thunk.h>
#include <device/framebuffer.h>
#include <util/array.h>
#include <util/pool.h>
#include <cpu/memory.h>

#define MAX_DEPTH 10000000

void freeThunk(Thunk*);

void memcpy(byte* dst_,byte *src_,int n) {
  dword *src = (void*)src_,*dst = (void*)dst_;
  dword* end = src+(n>>2);
  
  while(src != end) {
    *dst = *src; src++; dst++;
  }
  int i;
  for(i=0;i<(n&3);i++)
    ((byte*)dst)[i] = ((byte*)src)[i];
}
int strlen(char* s) {
  char* c = s;
  while(*c != '\0') c++;
  return c-s;
}

Pool linkPool = { 0, sizeof(Link) };
Pool thunkPool = { 0, sizeof(Thunk) };
Pool numPool = { 0, sizeof(Value) + sizeof(int) };
Value nilVal = { .shape = NIL };

void rebase(Thunk* t,int depth);
int parentDepth(Thunk* t);

void nothing(struct Thunk* _) { }
Thunk* newThunk() {
  Thunk* ret = poolAllocU(&thunkPool);
  ret->depth = 0;
  Link* ring = &ret->ring;
  ring->cRight = ring; 
  ring->pRight = ring; 
  ring->cLeft = ring;
  ring->pLeft = ring; 
  ret->pureVal = NULL;
  ret->initializer = nothing;
  return ret;
}

Value* number(int n) {
  Value* ret = poolAllocU(&numPool);
  ret->shape = NUMBER;

  int* v = (int*)&ret->data;
  *v = n;
  return ret;
}
Value* string(char* s) {
  int l = strlen(s);
  Value* ret = newArray(sizeof(Value) + sizeof(String) + l + 1);
  ret->shape = STRING;

  String* str = AFTER(ret);
  str->sz = l;
  memcpy(AFTER(str),(byte*)s,l+1);
   
  return ret;
}
Value* array(int n,...) {
  Thunk** args = AFTER(&n);
  Value* ret = newArray(sizeof(Value) + sizeof(Array) + n*sizeof(Thunk*));
  ret->shape = ARRAY;

  Array* arr = AFTER(ret);
  int i;
  arr->size = n;
  for(i=0;i<n;i++)
    arr->data[i] = args[i];

  return ret;
}
Value* func(Function f) {
  Value* ret = poolAllocU(&numPool);
  ret->shape = FUNCTION;
  Function* f2 = AFTER(ret);
  *f2 = f;
  return ret;
}
Value* nil() {
  return &nilVal;
}

void freeValue(Value* v) {
  switch(v->shape) {
  case NUMBER:
  case FUNCTION:
    poolFreeU(&numPool,v);
    break;
  case NIL:
    break;
  default:
    freeArray(v);
  }
}

void link(Thunk* f,Thunk* s) {
  Link* l = poolAllocU(&linkPool);
  l->down = s;
  ATTACH(l,&s->ring,p);
  
  l->up = f; 
  ATTACH(l,&f->ring,c);
}
Thunk* pure(Value* v) {
  Thunk* ret = newThunk();
  ret->state = PURE;
  ret->pureVal = v;
  if(v->shape == ARRAY) {
    int i;
    Array* arr = AFTER(v);
    for(i=0;i<arr->size;i++)
      link(ret,arr->data[i]);
  }
  return ret;
}
void evaluate(Thunk* t) {
  Thunk* child = t->ring.cRight->down;
  force(child);
  switch(child->pureVal->shape) {
  case ARRAY: {
    Array* arr = AFTER(&child->pureVal);
    Array* newArr = newArray(sizeof(Array) + arr->size*sizeof(arr->data[0]));
    
    int i;
    newArr->size = arr->size;
    for(i=0;i<arr->size;i++)
      newArr->data[i] = eval(arr->data[i]);
    
    Thunk* ft = newArr->data[0];
    force(ft);
    switch(ft->pureVal->shape) {
    case FUNCTION: {
      Function* f = AFTER(&ft->pureVal);
           
      Thunk* res = (*f)(newArr);
      force(res);
      t->pureVal = res->pureVal;
      res->pureVal = NULL;
      freeThunk(res);
      break;
    }
    default:
      t->pureVal = nil();
      break;
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
  if(parentDepth(t) > t->depth) {
    /* 
       If none of our parents lead to the root, then we are the root of a cycle 
       or have no parents.
    */
    Link *child;
    FORRING(child,t->ring,c) {
      DETACH(child,p);
      freeThunk(child->down);
    }
    while(t->ring.cRight != &t->ring) {
      child = t->ring.cRight;
      t->ring.cRight = child->cRight;
      poolFreeU(&linkPool,child);
    }
    if(t->pureVal != NULL && t->initializer != evaluate) freeValue(t->pureVal);
    poolFreeU(&thunkPool,t);
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
  case LOCKED:
    while(1); /* We loop for now when encountering a locked thunk. */
    break;
  }
}
void invalidate(Thunk* t) {
  Link* parent;
  FORRING(parent,t->ring,p) 
    if(parent->up->state == PURE) {
      parent->up->state = THUNK;
      invalidate(parent->up);
    }
}
int parentDepth(Thunk* t) {
  Link* l;
  int depth = MAX_DEPTH;
  FORRING(l,t->ring,p) 
    if(l->up->depth < depth)
      depth = l->up->depth;
  return depth+1;
}
void rebase(Thunk* t,int depth) {
  Link *child;
  t->depth = depth;
  FORRING(child,t->ring,c) 
    if(child->down->depth > depth+1)
      rebase(child->down,depth+1);
}
void replace(Thunk* old,Thunk* new) {
  invalidate(old);
  
  Link* parent;
  Link* next;
  for(parent = old->ring.pRight; parent != &old->ring;parent = next) {
    next = parent->pRight;
    DETACH(parent,p);
    ATTACH(parent,&new->ring,p);
    parent->down = new;
  }

  rebase(new,parentDepth(new));
  freeThunk(old);
}

void showVal(Value* v) {
  switch(v->shape) {
  case NUMBER: {
    int* n = AFTER(v);
    printf("NUMBER(%d)",*n);
    break;
  }
  case STRING: {
    String* str = AFTER(v);
    printf("STRING(%s)",str->data);
    break;
  }
  case ARRAY:
    printf("ARRAY");
    break;
  case FUNCTION:
    printf("FUNCTION");
    break;
  case NIL:
    printf("NIL");
  }
}

