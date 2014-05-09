#ifndef INCLUDED_UNIVERSE
#define INCLUDED_UNIVERSE

#include <cpu/descriptors.h>
#include <core/feature.h>

typedef int UIndex;
typedef struct Universe {
  Dir* pageDir;
  int dpl;
  UIndex index;
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

