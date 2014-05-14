#include <meXa/gear.h>
#include <device/framebuffer.h>
#include <util/array.h>
#include <util/pool.h>
#include <util/memory.h>
#include <meXa/dictionary.h>

#define MAX_DEPTH 10000000
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define NEXTDEPTH(n) min(MAX_DEPTH,(n)+1)
#define DEEPER(a,b) ((a) > (b) || (a) == MAX_DEPTH)

static void nothing(Gear*);
static void evaluate(Gear*);

Pool cogPool = { 0, sizeof(Cog) };
Pool gearPool = { 0, sizeof(Gear) };
struct {
  Torque v;
  Map m;
} rootMap = {
  { .unit = DICTIONARY },
  EMPTY
};
Gear rootGear = {
  .state = TORQUE,
  .depth = 0,
  .torque = &rootMap.v,
  .initializer = nothing,
  .ring = {
    .cRight = &rootGear.ring,
    .pRight = &rootGear.ring,
    .cLeft = &rootGear.ring,
    .pLeft = &rootGear.ring,
  }
};

static void debase(Gear* t,int depth);
static int parentDepth(Gear* t);
static void invalidate(Gear* t);

Gear* newGear() {
  Gear* ret = poolAllocU(&gearPool);
  Cog* ring = &ret->ring;
  ring->cRight = ring; 
  ring->pRight = ring; 
  ring->cLeft = ring;
  ring->pLeft = ring; 
  ret->depth = MAX_DEPTH;
  ret->torque = NULL;
  ret->initializer = nothing;
  return ret;
}
void freeGear(Gear* t) {
  if(DEEPER(t->depth,parentDepth(t))) {
    /* 
       If none of our parents lead to the root, then we are the root of a cycle 
       or have no parents.
    */
    Cog *child;
    FORRING(child,t->ring,c) {
      DETACH(child,p);
      debase(child->down,NEXTDEPTH(t->depth));
      freeGear(child->down);
    }
    while(t->ring.cRight != &t->ring) {
      child = t->ring.cRight;
      t->ring.cRight = child->cRight;
      poolFreeU(&cogPool,child);
    }
    if(!(t->torque == NULL
	 || (t->initializer == evaluate && t->torque->owned)))
      freeTorque(t->torque);
    poolFreeU(&gearPool,t);
  }
}

Gear* pure(Torque* v) {
  Gear* ret = newGear();
  ret->state = TORQUE;
  ret->torque = v;
  return ret;
}
Gear* transmit(Gear* t) {
  Gear* ret = newGear();
  ret->state = MESH;
  ret->initializer = evaluate;
  cog(ret,t);
  return ret;
}

Torque* torque(Gear* t) {
  switch(t->state) {
  case MESH: {
    t->state = LOCKED;
    t->initializer(t);
    t->state = TORQUE;
    break;
  }
  case LOCKED:
    printf("Locked block\n");
    while(1); /* We loop for now when encountering a locked gear. */
  default:
    break;
  }
  return t->torque;
}

void replace(Gear* old,Gear* new) {
  if(old != new) {
    invalidate(old);
  
    Cog* parent;
    Cog* next;
    for(parent = old->ring.pRight; parent != &old->ring;parent = next) {
      next = parent->pRight;
      DETACH(parent,p);
      ATTACH(parent,&new->ring,p);
      parent->down = new;
    }

    rebase(new,parentDepth(new));
    freeGear(old);
  }
}

Cog* cog(Gear* f,Gear* s) {
  Cog* l = poolAllocU(&cogPool);
  l->down = s;
  ATTACH(l,&s->ring,p);
  
  l->up = f; 
  ATTACH(l,&f->ring,c);
  
  return l;
}

static int parentDepth(Gear* t) {
  Cog* l;
  int depth = MAX_DEPTH;
  FORRING(l,t->ring,p) 
    if(l->up->depth < depth)
      depth = l->up->depth;
  return NEXTDEPTH(depth);
}

static void invalidate(Gear* t) {
  Cog* parent;
  FORRING(parent,t->ring,p) 
    if(parent->up->state == TORQUE) {
      parent->up->state = MESH;
      invalidate(parent->up);
    }
}
void rebase(Gear* t,int depth) {
  Cog *child;
  t->depth = depth;
  FORRING(child,t->ring,c) {
    int next = NEXTDEPTH(depth);
    if(DEEPER(child->down->depth,next))
      rebase(child->down,next);
  }
}
static void debase(Gear* t,int depth) {
  if(t->depth == depth) {
    Cog *child;
    t->depth = parentDepth(t);
    FORRING(child,t->ring,c) 
      debase(child->down,NEXTDEPTH(depth));
  }
}

static void nothing(Gear* _) { }
static void evaluate(Gear* t) {
  Gear* child = t->ring.cRight->down;
  Torque* val = torque(child);
  switch(val->unit) {
  case ARRAY: {
    Array* arr = AFTER(val);

    Gear* ft = arr->data[0]->down;
    Torque* ftVal = torque(ft);
    
    switch(ftVal->unit) {
    case FUNCTION: {
      Function* f = AFTER(ftVal);

      t->torque = (*f)(arr);
      break;
    }
    default:
      t->torque = nil();
      break;
    }
    break;
  }
  default:
    t->torque = child->torque;
    break;
  }
}

