#ifndef INCLUDED_VALUE
#define INCLUDED_VALUE

#include <util/memory.h>

typedef enum {
  NIL, NUMBER, STRING, ARRAY, DICTIONARY, FUNCTION
} Shape;
typedef struct {
  Shape shape:24; byte owned;
  byte data[0];
} PACKED Value; 

typedef struct {
  int sz; 
  char data[0];
} String;
typedef struct {
  int size;
  struct Link* data[0];
} Array;
typedef Value* (*Function)(Array* arr);

Value* number(int);
Value* string(char*);
Value* array(int n,...);
Value* func(Function);
Value* nil();
Value* dictionary();

void freeValue(Value*);

void showVal(Value*);

#endif
