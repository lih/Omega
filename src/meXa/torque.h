#ifndef INCLUDED_VALUE
#define INCLUDED_VALUE

#include <util/memory.h>

typedef enum {
  NIL, NUMBER, STRING, ARRAY, DICTIONARY, FUNCTION
} Unit;
typedef struct {
  Unit unit:24; byte owned;
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
typedef Torque* (*Function)(Array* arr);

Torque* number(int);
Torque* string(char*);
Torque* array(int n,...);
Torque* func(Function);
Torque* nil();
Torque* dictionary();

void freeTorque(Torque*);

void showTorque(Torque*);

#endif
