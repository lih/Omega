#include <init/repl/map.h>
#include <util/pool.h>
#include <device/framebuffer.h>
#include <util/memory.h>
#include <util/array.h>

MapNode voidNode = {
  .left = EMPTY, .right = EMPTY, .height = 0
};
static Pool nodePool = { 0, sizeof(MapNode) };

static MapNode* getNode(Map* m,char* key);

int strcmp(char* s1,char* s2) {
  while(*s1 == *s2 && *s1 != '\0')
    { s1++; s2++; }
  
  return *s2-*s1;
}

void define(char* key,Thunk* new) {
  Thunk* old = lookup(&rootThunk,key);
  replace(old,new);
}

Thunk* lookup(Thunk* map,char* key) {
  Value* v = force(map);
  if(v->shape == DICTIONARY) {
    Map* root = AFTER(v);
    MapNode* n = getNode(root,key);
    if(n->link == NULL) 
      n->link = link(map,pure(nil()));
    return n->link->down;
  }
  else {
    printf("Thunk is not a dictionary.\n");
    return pure(nil());
  }
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
MapNode* getNode(Map* root,char* key) {
#define rootp (*root)
  if(rootp == EMPTY) {
    rootp = poolAllocU(&nodePool);
    int l = strlen(key);
    char* newKey = newArray(l+1);
    memcpy(newKey,key,l+1);
    SET_STRUCT(MapNode,*rootp,{ 
	.key = newKey, .left = &voidNode, .right = &voidNode, .height = 1,
	  .link = NULL
	  });
        
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
    printf("%s:%d:",root->key,root->height); showVal(root->link->down->pureVal);
    showMap(root->right);
    putChar('\\');
  }
}

