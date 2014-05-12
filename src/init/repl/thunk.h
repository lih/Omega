#ifndef INCLUDED_THUNK
#define INCLUDED_THUNK

#include <util/memory.h>
#include <init/repl/value.h>

typedef struct Link {
  struct Link *pLeft,*pRight;
  struct Link *cLeft,*cRight;
  struct Thunk *up,*down;
} Link;
typedef struct Thunk {
  enum {
    THUNK, LOCKED, PURE
  } state;
  void (*initializer)(struct Thunk*);
  Value* pureVal;
  int depth;
  Link ring;
} Thunk;
#define FORRING(i,r,t) for(i=(r).t##Right ; i != &(r);i=i->t##Right )
#define DETACH(l,t) {				\
    (l)->t##Left->t##Right = (l)->t##Right;	\
    (l)->t##Right->t##Left = (l)->t##Left;	\
  }
#define ATTACH(a,b,t) {				\
    (a)->t##Right = (b);			\
      (a)->t##Left = (b)->t##Left;		\
	(b)->t##Left = (a);			\
	  (a)->t##Left->t##Right = (a);		\
  }

extern Thunk rootThunk;

Thunk* pure(Value*);
Thunk* eval(Thunk*);
void freeThunk(Thunk*);

Value* force(Thunk*);
void replace(Thunk* old, Thunk* new);

Link* link(Thunk* father,Thunk* son);
void rebase(Thunk* t, int n);

#endif
