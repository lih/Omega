#include <init/repl/value.h>
#include <util/array.h>
#include <util/pool.h>
#include <init/repl/thunk.h>
#include <device/framebuffer.h>
#include <init/repl/map.h>

Pool numPool = { 0, sizeof(Value) + sizeof(int) };
Value nilVal = { .shape = NIL };

void freeValue(Value* v) {
  switch(v->shape) {
  case NUMBER:
  case FUNCTION:
  case DICTIONARY:
    poolFreeU(&numPool,v);
    break;
  case NIL:
    break;
  default:
    freeArray(v);
  }
}

Value* number(int n) {
  Value* ret = poolAllocU(&numPool);
  ret->shape = NUMBER;
  ret->copy = 0;

  int* v = (int*)&ret->data;
  *v = n;
  return ret;
}
Value* string(char* s) {
  int l = strlen(s);
  Value* ret = newArray(sizeof(Value) + sizeof(String) + l + 1);
  ret->shape = STRING;
  ret->copy = 0;

  String* str = AFTER(ret);
  str->sz = l;
  memcpy(AFTER(str),(byte*)s,l+1);
   
  return ret;
}
Value* array(int n,...) {
  Thunk** args = AFTER(&n);
  Value* ret = newArray(sizeof(Value) + sizeof(Array) + n*sizeof(Thunk*));
  ret->shape = ARRAY;
  ret->copy = 0;

  Array* arr = AFTER(ret);
  int i;
  arr->size = n;
  for(i=0;i<n;i++)
    arr->data[i] = args[i];

  return ret;
}
Value* func(Function f) {
  Value* ret = poolAllocU(&numPool);
  ret->shape = FUNCTION;
  ret->copy = 0;
  Function* f2 = AFTER(ret);
  *f2 = f;
  return ret;
}
Value* nil() {
  return &nilVal;
}
Value* dictionary() {
  Value* ret = poolAllocU(&numPool);
  ret->shape = DICTIONARY;
  ret->copy = 0;
  Map* m = AFTER(ret);
  *m = EMPTY;
  return ret;
}

void showVal(Value* v) {
  switch(v->shape) {
  case NUMBER: {
    int* n = AFTER(v);
    printf("NUMBER(%d)",*n);
    break;
  }
  case STRING: {
    String* str = AFTER(v);
    printf("STRING(%s)",str->data);
    break;
  }
  case ARRAY: {
    putChar('[');
    int i;
    Array* arr = AFTER(v);
    for(i=0;i<arr->size;i++) {
      if(i!=0) putChar(' ');
      showVal(force(arr->data[i]));
    }
    putChar(']');
    break;
  }
  case FUNCTION:
    printf("FUNCTION");
    break;
  case DICTIONARY:
    printf("DICTIONARY");
    break;
  case NIL:
    printf("NIL");
  }
}

