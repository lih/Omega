#include <meXa/gear.h>
#include <device/framebuffer.h>
#include <util/array.h>
#include <util/pool.h>
#include <util/memory.h>
#include <meXa/dictionary.h>

#define MAX_DEPTH 123456789
#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define NEXTDEPTH(n) min(MAX_DEPTH,(n)+1)

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

static int parentDepth(Gear* t);
static void invalidate(Gear* t);

Gear* newGear() {
  Gear* ret = poolAlloc(&gearPool);
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
  int pdepth = parentDepth(t);
  if(pdepth > t->depth || t->depth == MAX_DEPTH) {
    /* 
       If none of our parents lead to the root, then we are the root of a cycle 
       or have no parents.
    */
    INFO("Freeing gear %x (depth: %x, pdepth: %x)\n",t,t->depth,parentDepth(t));
    
    Cog *child;
    FORRING(child,t->ring,c) {
      DETACH(child,p);
      debase(child->down,NEXTDEPTH(t->depth));
      freeGear(child->down);
    }
    while(t->ring.cRight != &t->ring) {
      child = t->ring.cRight;
      t->ring.cRight = child->cRight;
      poolFree(&cogPool,child);
    }
    if(t->torque != NULL)
      freeTorque(t->torque);
    poolFree(&gearPool,t);

  }
}

Gear* pure(Torque* v) {
  Gear* ret = newGear();
  ret->state = TORQUE;
  ret->torque = v;
  return ret;
}
Gear* mesh(Gear* t) {
  Gear* ret = newGear();
  ret->state = MESH;
  ret->initializer = evaluate;
  link(ret,t);
  ret->torque = cog(link(ret,pure(nil())));
  return ret;
}

Torque* force(Gear* t) {
  switch(t->state) {
  case MESH: {
    t->state = LOCKED;
    t->initializer(t);
    t->state = TORQUE;
    break;
  }
  case LOCKED:
    WARN("Locked block\n");
    while(1); /* We loop for now when encountering a locked gear. */
  default:
    break;
  }
  return t->torque;
}
Torque* torque(Gear* g) {
  Torque* t = force(g);
  while(t->unit == COG) {
    Cog** c = (Cog**)AFTER(t);
    g = (*c)->down;
    t = force(g);
  }
  return t;
}

void replace(Gear* old,Gear* new) {
  if(old != new) {
    invalidate(old);
    INFO("Replacing %x by %x\n",old,new);

    Cog* parent;
    Cog* next;
    for(parent = old->ring.pRight; parent != &old->ring;parent = next) {
      next = parent->pRight;
      DETACH(parent,p);
      ATTACH(parent,&new->ring,p);
      parent->down = new;
    }
    
    rebase(new,old->depth);
    debase(old,old->depth);
    freeGear(old);
  }
}

Cog* link(Gear* f,Gear* s) {
  Cog* l = poolAlloc(&cogPool);
  l->down = s;
  l->up = f; 

  ATTACH(l,&s->ring,p);
  ATTACH(l,&f->ring,c);
  rebase(s,NEXTDEPTH(f->depth));

  return l;
}
void unlink(Cog* c) {
  CHATTER("Unlinking %x from %x\n",c->down,c->up);

  DETACH(c,c);
  DETACH(c,p);
  debase(c->down,NEXTDEPTH(c->up->depth));

  freeGear(c->down);
  poolFree(&cogPool,c);
}

static int parentDepth(Gear* t) {
  Cog* l;
  int depth = MAX_DEPTH;
  FORRING(l,t->ring,p) depth = min(depth,l->up->depth);
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
void rebase(Gear* t,int newdepth) {
  if(t->depth > newdepth) {
    CHATTER("Rebasing gear %x to depth %d (from %d)\n",t,newdepth,t->depth);
    Cog *child;
    t->depth = newdepth;
    FORRING(child,t->ring,c) 
      rebase(child->down,NEXTDEPTH(newdepth));
  }
}
void debase(Gear* t,int olddepth) {
  if(t->depth == olddepth) {
    Cog *child;
    t->depth = parentDepth(t);
    CHATTER("Debasing gear %x from depth %d (to %d)\n",t,olddepth,t->depth);
    FORRING(child,t->ring,c) 
      debase(child->down,NEXTDEPTH(olddepth));
  }
}

static void nothing(Gear* _) { }
static void evaluate(Gear* t) {
  Gear* child = t->ring.cRight->down;
  Torque* val = force(child);
  Cog** c = AFTER(t->torque);
  unlink(*c);
  
  switch(val->unit) {
  case ARRAY: {
    Array* arr = AFTER(val);

    Gear* ft = arr->data[0]->down;
    Torque* ftVal = torque(ft);

    switch(ftVal->unit) {
    case FUNCTION: {
      Function* f = AFTER(ftVal);

      *c = link(t,(*f)(arr));
      break;
    }
    default:
      *c = link(t,pure(nil()));
      break;
    }
    break;
  }
  default:
    *c = link(t,child);
    break;
  }
}

static Gear* instance(Gear*);

static void nodeInstance(MapNode* n,void* dict) {
  Gear* m = dict;
  replace(lookup(m,n->key),instance(n->cog->down));
}
static Gear* instance(Gear* g) {
  switch(g->torque->unit) {
  case ABSTRACT: {
    Cog** sub = AFTER(g->torque);
    if(*sub != NULL) {
      Gear* subG = (*sub)->down;
      if(subG->torque->unit == ABSTRACT) {
	Gear* ret = pure(nil());
	ret->torque = abstract(link(ret,instance(subG)));
	return ret;
      }
      else
	return subG;
    }
    else
      return pure(abstract(NULL));
  }
  case ARRAY: {
    Array* oldA = AFTER(g->torque);
    Torque *newT = newArray(sizeof(Torque) + sizeof(Array) + oldA->size*sizeof(Cog*)); {
      newT->unit = ARRAY;
    }
    Array* newA = AFTER(newT); {
      newA->size = oldA->size;
    }

    Gear* ret = pure(newT);
    DOTIMES(i,newA->size)
      newA->data[i] = link(ret,instance(oldA->data[i]->down));

    return ret;
  }
  case DICTIONARY: {
    Gear* ret = pure(dictionary());
    forNodes(*(Map*)AFTER(g->torque),ret,nodeInstance);
    return ret;
  }
  case COG: {
    Gear* form = g->ring.cRight->down;
    return mesh(instance(form));
  }
  case FUNCTION: {
    
  }
  case NIL:
  case NUMBER:
  case STRING:
    return g;
  }
  return NULL;
}
Gear* instanciate(Array* args) {
  Gear* f = args->data[0]->down;
  Cog* env = f->ring.cRight;
  Gear* tpl = env->down;
  env = env->cRight;
  Gear* vargs = env->down;

  Array* va = AFTER(vargs->torque);
  DOTIMES(i,va->size) {
    Cog** c = AFTER(va->data[i]->down->torque);
    (*c) = link(vargs,args->data[i+1]->down);
  }
  Gear* ret = instance(tpl);
  rebase(ret,tpl->depth);
  DOTIMES(j,va->size) {
    Cog** c = AFTER(va->data[j]->down->torque);
    unlink(*c);
    *c = NULL;
  }

  return ret;
}
