#ifndef INCLUDED_UNIVERSE
#define INCLUDED_UNIVERSE

#include <x86/descriptors.h>
#include <util/feature.h>

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
void      freeUniverse(Universe*);

void      mapPage(Universe* univ,void* vpage,void* page,byte rw);
DirEntry* dirVal(Dir*,void*);

#endif

