#ifndef INCLUDED_THUNK
#define INCLUDED_THUNK

#include <cpu/memory.h>

typedef enum {
  NUMBER, STRING, ARRAY, FUNCTION, NIL
} Shape;
typedef struct {
  Shape shape;
  byte data[0];
} Value; 
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
  int depth;
  Value* pureVal;
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

typedef struct {
  int sz; 
  char data[0];
} String;
typedef struct {
  int size;
  Thunk* data[0];
} Array;
typedef Thunk* (*Function)(Array* arr);

Value* number(int);
Value* string(char*);
Value* array(int n,...);
Value* func(Function);
Value* nil();

Thunk* pure(Value*);
Thunk* eval(Thunk*);
void force(Thunk*);
void replace(Thunk* old, Thunk* new);

void freeThunk(Thunk*);

void showVal(Value*);

#endif
