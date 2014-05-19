#ifndef INCLUDED_VALUE
#define INCLUDED_VALUE

#include <util/memory.h>

typedef enum {
  NIL, NUMBER, STRING, ARRAY, DICTIONARY, FUNCTION, COG, ABSTRACT
} Unit;
typedef struct {
  Unit unit;
  byte data[0];
} PACKED Torque; 

typedef struct {
  int sz; 
  char data[0];
} String;
typedef struct {
  int size;
  struct Cog* data[0];
} Array;
typedef struct Gear* (*Function)(Array* arr);

Torque* number(int);
Torque* string(char*);
Torque* array(int n,struct Gear* parent,struct Gear** children);
Torque* arrayn(int n,struct Gear* parent,...);
Torque* func(Function);
Torque* nil();
Torque* dictionary();
Torque* cog(struct Cog*);
Torque* abstract(struct Cog*);

void freeTorque(Torque*);

void showTorque(Torque*);

#endif
