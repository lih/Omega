#ifndef INCLUDED_MESH
#define INCLUDED_MESH

#include <util/memory.h>
#include <meXa/torque.h>

typedef struct Cog {
  struct Cog *pLeft,*pRight;
  struct Cog *cLeft,*cRight;
  struct Gear *up,*down;
} Cog;
typedef struct Gear {
  enum {
    MESH, LOCKED, TORQUE
  } state;
  void (*initializer)(struct Gear*);
  Torque* torque;
  int depth;
  Cog ring;
} Gear;
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

extern Gear rootGear;

Gear* pure(Torque*);
Gear* transmit(Gear*);
Gear* newGear();
void freeGear(Gear*);

Torque* torque(Gear*);
void replace(Gear* old, Gear* new);

Cog* cog(Gear* father,Gear* son);
void rebase(Gear* t, int n);

#endif
