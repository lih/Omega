#ifndef INCLUDED_MAP
#define INCLUDED_MAP

#include <init/repl/thunk.h>

typedef struct MapNode {
  char* key;
  Thunk* t;
  int height;
  struct MapNode *left,*right;
} MapNode;
typedef MapNode *Map;

extern MapNode voidNode;
extern Map rootNode;

#define EMPTY (&voidNode)

MapNode* getNode(Map* rootNode,char* key);
void showMap(Map map);

void define(char* key,Thunk* val);

#endif
