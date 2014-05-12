#ifndef INCLUDED_MAP
#define INCLUDED_MAP

#include <init/repl/thunk.h>

typedef struct MapNode {
  char* key;
  Link* link;
  int height;
  struct MapNode *left,*right;
} MapNode;
typedef MapNode *Map;

extern MapNode voidNode;
#define EMPTY (&voidNode)

Thunk* lookup(Thunk* map,char* key);

void showMap(Map map);

void define(char* key,Thunk* val);

#endif
