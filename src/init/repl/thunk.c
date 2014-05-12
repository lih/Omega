#include <init/repl/thunk.h>
#include <device/framebuffer.h>
#include <util/array.h>
#include <util/pool.h>
#include <util/memory.h>

#define MAX_DEPTH 10000000
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define NEXTDEPTH(n) min(MAX_DEPTH,(n)+1)

Pool linkPool = { 0, sizeof(Link) };
Pool thunkPool = { 0, sizeof(Thunk) };
Thunk rootThunk = {
  .state = PURE,
  .depth = 0,
  .pureVal = NULL,
  .ring = {
    .cRight = &rootThunk.ring,
    .pRight = &rootThunk.ring,
    .cLeft = &rootThunk.ring,
    .pLeft = &rootThunk.ring,
  }
};

static void debase(Thunk* t,int depth);
static int parentDepth(Thunk* t);
static void invalidate(Thunk* t);

static void nothing(Thunk*);
static void mkArray(Thunk*);
static void evaluate(Thunk*);

Thunk* newThunk() {
  Thunk* ret = poolAllocU(&thunkPool);
  Link* ring = &ret->ring;
  ring->cRight = ring; 
  ring->pRight = ring; 
  ring->cLeft = ring;
  ring->pLeft = ring; 
  ret->depth = MAX_DEPTH;
  ret->pureVal = NULL;
  ret->initializer = nothing;
  return ret;
}
void freeThunk(Thunk* t) {
  if(parentDepth(t) > t->depth || t->depth >= MAX_DEPTH) {
    /* 
       If none of our parents lead to the root, then we are the root of a cycle 
       or have no parents.
    */
    printf("Freeing thunk at %x (depth %d)\n",t,t->depth);

    Link *child;
    FORRING(child,t->ring,c) {
      DETACH(child,p);
      debase(child->down,NEXTDEPTH(t->depth));
      freeThunk(child->down);
    }
    while(t->ring.cRight != &t->ring) {
      child = t->ring.cRight;
      t->ring.cRight = child->cRight;
      poolFreeU(&linkPool,child);
    }
    if(t->pureVal != NULL
       && (t->initializer != evaluate
	   || !(t->pureVal->copy))) freeValue(t->pureVal);
    poolFreeU(&thunkPool,t);
  }
}

Thunk* pure(Value* v) {
  Thunk* ret = newThunk();
  ret->state = PURE;
  ret->pureVal = v;
  if(v->shape == ARRAY) {
    int i;
    Array* arr = AFTER(v);
    ret->initializer = mkArray;
    for(i=0;i<arr->size;i++) 
      link(ret,arr->data[i]);
  }
  return ret;
}
Thunk* eval(Thunk* t) {
  Thunk* ret = newThunk();
  ret->state = THUNK;
  ret->initializer = evaluate;
  link(ret,t);
  return ret;
}

Value* force(Thunk* t) {
  switch(t->state) {
  case THUNK: {
    t->state = LOCKED;
    t->initializer(t);
    t->state = PURE;
    break;
  }
  case LOCKED:
    printf("Locked block\n");
    while(1); /* We loop for now when encountering a locked thunk. */
  default:
    break;
  }
  return t->pureVal;
}

void plant(Thunk* t) {
  link(&rootThunk,t);
  rebase(t,1);
}

void replace(Thunk* old,Thunk* new) {
  if(old != new) {
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
}

void link(Thunk* f,Thunk* s) {
  Link* l = poolAllocU(&linkPool);
  l->down = s;
  ATTACH(l,&s->ring,p);
  
  l->up = f; 
  ATTACH(l,&f->ring,c);
}

static int parentDepth(Thunk* t) {
  Link* l;
  int depth = MAX_DEPTH;
  FORRING(l,t->ring,p) 
    if(l->up->depth < depth)
      depth = l->up->depth;
  return NEXTDEPTH(depth);
}

static void invalidate(Thunk* t) {
  Link* parent;
  FORRING(parent,t->ring,p) 
    if(parent->up->state == PURE) {
      parent->up->state = THUNK;
      invalidate(parent->up);
    }
}
void rebase(Thunk* t,int depth) {
  Link *child;
  t->depth = depth;
  FORRING(child,t->ring,c) 
    if(child->down->depth > NEXTDEPTH(depth))
      rebase(child->down,NEXTDEPTH(depth));
}
static void debase(Thunk* t,int depth) {
  if(t->depth == depth) {
    Link *child;
    t->depth = parentDepth(t);
    FORRING(child,t->ring,c) 
      debase(child->down,NEXTDEPTH(depth));
  }
}

static void nothing(Thunk* _) { }
static void mkArray(Thunk* t) {
  Array* arr = AFTER(t->pureVal);
  int n=0;
  Link* child;
  FORRING(child,t->ring,c)
    arr->data[n++] = child->down;
}
static void evaluate(Thunk* t) {
  Thunk* child = t->ring.cRight->down;
  Value* val = force(child);
  switch(val->shape) {
  case ARRAY: {
    Array* arr = AFTER(val);
    
    Thunk* ft = arr->data[0];
    Value* ftVal = force(ft);
    switch(ftVal->shape) {
    case FUNCTION: {
      Function* f = AFTER(ftVal);
      t->pureVal = (*f)(arr);
      break;
    }
    default:
      t->pureVal = nil();
      break;
    }
    break;
  }
  default:
    t->pureVal = child->pureVal;
    t->pureVal->copy = 1;
    break;
  }
}

