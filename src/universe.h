#ifndef INCLUDED_UNIVERSE
#define INCLUDED_UNIVERSE

#include "descriptors.h"
#include "feature.h"

typedef struct Universe {
  Dir* pageDir;
  int dpl;
  int index;
  struct Universe *up, *left, *right, *down;  
} Universe;

extern Feature _universe_;

extern Universe kernelSpace;
extern Universe rootSpace;

#define F_ADDR(n) ((DirEntry*)(((dword)n)&0xfffff000))

Universe* newUniverse(Universe* father);

void      mapPage(Universe* univ,dword vpage,void* page,byte rw);
DirEntry* dirVal(Dir*,dword);

#endif

