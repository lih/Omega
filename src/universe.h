#ifndef INCLUDED_UNIVERSE
#define INCLUDED_UNIVERSE

typedef dword DirEntry;
#define DIR_SIZE (PAGE_SIZE/sizeof(DirEntry))
typedef dword Dir[DIR_SIZE];

typedef struct {
  Dir* pageDir;
  int dpl;
} Universe;

extern Universe kernelSpace;

Universe* newUniverse();

void initUniverse();
void mapPage(Universe* univ,dword vpage,void* page);

#endif

