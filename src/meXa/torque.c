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
  case COG:
    poolFree(&numPool,v);
    break;
  case NIL:
    break;
  default:
    freeArray(v);
  }
}

Torque* number(int n) {
  Torque* ret = poolAlloc(&numPool);
  ret->unit = NUMBER;

  int* v = (int*)&ret->data;
  *v = n;
  return ret;
}
Torque* cog(Cog* c) {
  Torque* ret = poolAlloc(&numPool);
  ret->unit = COG;

  Cog** v = (Cog**)&ret->data;
  *v = c;
  return ret;
}
Torque* abstract(Cog* c) {
  Torque* ret = poolAlloc(&numPool);
  ret->unit = ABSTRACT;

  Cog** v = (Cog**)&ret->data;
  *v = c;
  return ret;
}
Torque* string(char* s) {
  int l = strlen(s);
  Torque* ret = newArray(sizeof(Torque) + sizeof(String) + l + 1);
  ret->unit = STRING;

  String* str = AFTER(ret);
  str->sz = l;
  memcpy(AFTER(str),(byte*)s,l+1);
   
  return ret;
}
Torque* array(int n,Gear* parent,Gear** children) {
  Torque* ret = newArray(sizeof(Torque) + sizeof(Array) + n*sizeof(Cog*));
  ret->unit = ARRAY;

  Array* arr = AFTER(ret);
  int i;
  arr->size = n;
  for(i=0;i<n;i++)
    arr->data[i] = link(parent,children[i]);

  return ret;
}
Torque* arrayn(int n,Gear* parent,...) {
  return array(n,parent,AFTER(&parent));
}
Torque* func(Function f) {
  Torque* ret = poolAlloc(&numPool);
  ret->unit = FUNCTION;
  Function* f2 = AFTER(ret);
  *f2 = f;
  return ret;
}
Torque* nil() {
  return &nilVal;
}
Torque* dictionary() {
  Torque* ret = poolAlloc(&numPool);
  ret->unit = DICTIONARY;
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
      showTorque(force(arr->data[i]->down));
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
  case COG:
    printf("COG");
    break;
  case ABSTRACT:
    printf("ABSTRACT");
    break;
  case NIL:
    printf("NIL");
  }
}

