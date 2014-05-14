#include <meXa/torque.h>
#include <util/array.h>
#include <util/pool.h>
#include <meXa/gear.h>
#include <device/framebuffer.h>
#include <meXa/dictionary.h>

Pool numPool = { 0, sizeof(Torque) + sizeof(int) };
Torque nilVal = { .unit = NIL };

void freeTorque(Torque* v) {
  switch(v->unit) {
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

Torque* number(int n) {
  Torque* ret = poolAllocU(&numPool);
  ret->unit = NUMBER;
  ret->owned = 1;

  int* v = (int*)&ret->data;
  *v = n;
  return ret;
}
Torque* string(char* s) {
  int l = strlen(s);
  Torque* ret = newArray(sizeof(Torque) + sizeof(String) + l + 1);
  ret->unit = STRING;
  ret->owned = 1;

  String* str = AFTER(ret);
  str->sz = l;
  memcpy(AFTER(str),(byte*)s,l+1);
   
  return ret;
}
Torque* array(int n,...) {
  Cog** args = AFTER(&n);
  Torque* ret = newArray(sizeof(Torque) + sizeof(Array) + n*sizeof(Gear*));
  ret->unit = ARRAY;
  ret->owned = 1;

  Array* arr = AFTER(ret);
  int i;
  arr->size = n;
  for(i=0;i<n;i++)
    arr->data[i] = args[i];

  return ret;
}
Torque* func(Function f) {
  Torque* ret = poolAllocU(&numPool);
  ret->unit = FUNCTION;
  ret->owned = 1;
  Function* f2 = AFTER(ret);
  *f2 = f;
  return ret;
}
Torque* nil() {
  return &nilVal;
}
Torque* dictionary() {
  Torque* ret = poolAllocU(&numPool);
  ret->unit = DICTIONARY;
  ret->owned = 1;
  Map* m = AFTER(ret);
  *m = EMPTY;
  return ret;
}

void showTorque(Torque* v) {
  switch(v->unit) {
  case NUMBER: {
    int* n = AFTER(v);
    printf("%d",*n);
    break;
  }
  case STRING: {
    String* str = AFTER(v);
    printf("\"%s\"",str->data);
    break;
  }
  case ARRAY: {
    putChar('[');
    int i;
    Array* arr = AFTER(v);
    for(i=0;i<arr->size;i++) {
      if(i!=0) putChar(' ');
      showTorque(torque(arr->data[i]->down));
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

