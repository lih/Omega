#include <init/repl/map.h>
#include <util/pool.h>
#include <device/framebuffer.h>
#include <util/memory.h>
#include <util/array.h>

MapNode voidNode = {
  .left = EMPTY, .right = EMPTY, .height = 0
};
Map rootNode = EMPTY;
Pool nodePool = { 0, sizeof(MapNode) };

int strcmp(char* s1,char* s2) {
  while(*s1 == *s2 && *s1 != '\0')
    { s1++; s2++; }
  
  return *s2-*s1;
}

#define map (*_map)
void rotr(Map* _map) {
  MapNode* mid = map->left->right;
  map->left->right = map;
  map = map->left;
  map->right->left = mid;
}
void rotl(Map* _map) {
  MapNode* mid = map->right->left;
  map->right->left = map;
  map = map->right;
  map->left->right = mid;
}
#undef map

void setHeight(Map n) {
  if(n->left->height > n->right->height)
    n->height = n->left->height+1;
  else 
    n->height = n->right->height+1;
}

void define(char* key,Thunk* t) {
  MapNode* n = getNode(&rootNode,key);
  replace(n->t,t);
  n->t = t;
}

MapNode* getNode(MapNode** root,char* key) {
#define rootp (*root)
  if(rootp == EMPTY) {
    rootp = poolAllocU(&nodePool);
    int l = strlen(key);
    char* newKey = newArray(l+1);
    memcpy(newKey,key,l+1);
    SET_STRUCT(MapNode,*rootp,{ 
	.key = newKey, .left = &voidNode, .right = &voidNode, .height = 1, .t = pure(nil())
	  });

    plant(rootp->t);
    return rootp;
  }

  int cmp = strcmp(rootp->key,key);
  if(cmp == 0)
    return rootp;
  else if(cmp > 0) {
    MapNode* ret = getNode(&rootp->right,key);
    if(rootp->right->height > rootp->left->height+1) {
      if(rootp->right->left->height > rootp->right->right->height)
	rotr(&rootp->right);
      rotl(root);
      setHeight(rootp->right);
      setHeight(rootp->left);
    }
    setHeight(rootp);
    return ret;
  }
  else {
    MapNode* ret = getNode(&rootp->left,key);
    if(rootp->left->height > rootp->right->height+1) {
      if(rootp->left->right->height > rootp->left->left->height)
	rotl(&rootp->left);
      rotr(root);
      setHeight(rootp->left);
      setHeight(rootp->right);
    }
    setHeight(rootp);
    return ret;
  }
#undef rootp
}

void showMap(MapNode* root) {
  if(root != &voidNode) {
    putChar('/');
    showMap(root->left);
    printf("%s:%d:",root->key,root->height); showVal(root->t->pureVal);
    showMap(root->right);
    putChar('\\');
  }
}

