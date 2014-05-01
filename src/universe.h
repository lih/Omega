#ifndef INCLUDED_UNIVERSE
#define INCLUDED_UNIVERSE

#include "descriptors.h"
#include "feature.h"

typedef struct {
  Dir* pageDir;
  int dpl;
} Universe;

extern Feature _universe_;

extern Universe kernelSpace;
extern Universe rootSpace;

Universe* newUniverse();

void      mapPage(Universe* univ,dword vpage,void* page,byte rw);
DirEntry* dirVal(Dir*,dword);

#endif

