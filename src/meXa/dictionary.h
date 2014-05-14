#ifndef INCLUDED_MAP
#define INCLUDED_MAP

#include <meXa/gear.h>

typedef struct MapNode {
  char* key;
  Cog* cog;
  int height;
  struct MapNode *left,*right;
} MapNode;
typedef MapNode *Map;

extern MapNode voidNode;
#define EMPTY (&voidNode)

Gear* lookup(Gear* map,char* key);

void showMap(Map map);

void define(char* key,Gear* val);

#endif
