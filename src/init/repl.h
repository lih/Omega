#ifndef INCLUDE_REPL
#define INCLUDE_REPL

#include <cpu/memory.h>

typedef struct {
  enum {
    NUMBER, STRING, ARRAY, FUNCTION
  } shape;
  byte data[0];
} Value; 
typedef struct Link {
  struct Link *leftUp,*rightUp;
  struct Link *leftDown,*rightDown;
  struct Thunk *up,*down;
} Link;
typedef struct Thunk {
  enum {
    THUNK, LOCKED, PURE
  } state;
  void (*initializer)(struct Thunk*);
  int depth;
  Value* pureVal;
  Link parentRing,childRing;
} Thunk;

typedef struct {
  int sz; 
  char data[0];
} String;
typedef struct {
  int size;
  Thunk* data[0];
} Array;
typedef Thunk* (*Function)(Array* arr);

void repl();

Value* number(int);
Value* string(char*);
Value* array(int n,...);
Value* func(Function);

Thunk* pure(Value*);
Thunk* eval(Thunk*);
void force(Thunk*);
void replace(Thunk* old, Thunk* new);

#endif
