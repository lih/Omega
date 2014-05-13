#include <init/repl/thunk.h>
#include <device/framebuffer.h>
#include <util/array.h>
#include <util/pool.h>
#include <util/memory.h>
#include <init/repl/map.h>

#define MAX_DEPTH 10000000
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define NEXTDEPTH(n) min(MAX_DEPTH,(n)+1)
#define DEEPER(a,b) ((a) > (b) || (a) == MAX_DEPTH)

static void nothing(Thunk*);
static void evaluate(Thunk*);

Pool linkPool = { 0, sizeof(Link) };
Pool thunkPool = { 0, sizeof(Thunk) };
struct {
  Value v;
  Map m;
} rootMap = {
  { .shape = DICTIONARY },
  EMPTY
};
Thunk rootThunk = {
  .state = PURE,
  .depth = 0,
  .pureVal = &rootMap.v,
  .initializer = nothing,
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
  if(DEEPER(t->depth,parentDepth(t))) {
    /* 
       If none of our parents lead to the root, then we are the root of a cycle 
       or have no parents.
    */
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
    if(!(t->pureVal == NULL
	 || (t->initializer == evaluate && t->pureVal->owned)))
      freeValue(t->pureVal);
    poolFreeU(&thunkPool,t);
  }
}

Thunk* pure(Value* v) {
  Thunk* ret = newThunk();
  ret->state = PURE;
  ret->pureVal = v;
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

Link* link(Thunk* f,Thunk* s) {
  Link* l = poolAllocU(&linkPool);
  l->down = s;
  ATTACH(l,&s->ring,p);
  
  l->up = f; 
  ATTACH(l,&f->ring,c);
  
  return l;
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
  FORRING(child,t->ring,c) {
    int next = NEXTDEPTH(depth);
    if(DEEPER(child->down->depth,next))
      rebase(child->down,next);
  }
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
static void evaluate(Thunk* t) {
  Thunk* child = t->ring.cRight->down;
  Value* val = force(child);
  switch(val->shape) {
  case ARRAY: {
    Array* arr = AFTER(val);

    Thunk* ft = arr->data[0]->down;
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
    break;
  }
}

