#include "constants.h"
#include "memory.h"
#include "universe.h"
#include "schedule.h"

#define F_PRESENT 1
#define F_RW      2
#define F_USER    4
#define F_PWT     8 		/* Page write-through */
#define F_CD      16 		/* Cache disabled */
#define F_ACCESSED 32
#define F_4M      128 		/* 4M pages instead of 4k */
#define F_GLOBAL  256

#define F_ADDR(n) ((DirEntry*)(((dword)n)&0xfffffc00))

#define PD_FLAGS (F_PRESENT | F_RW | F_USER)
#define PT_FLAGS(rw) (F_PRESENT | ((rw)? F_RW:0) | F_USER)

Dir* pageCounters;
Universe kernelSpace;
Pool univPool = { NULL, sizeof(Universe) };

Dir* newDir() {
  Dir* ret = allocatePage();
  int i;
  for(i=0;i<DIR_SIZE;i++)
    (*ret)[i] = NULL;
  return ret;
}
Universe* newUniverse() {
  require(&_memory_);
  
  Universe* ret = poolAlloc(&univPool);
  ret->pageDir = newDir();
  
  return ret;
}

DirEntry* dirVal(Dir* dir,dword vpage) {
  word dirind = vpage >> 22;
  word pageind = (vpage >> 12) & 0x3ff;
  DirEntry* pgDir = (*dir) + dirind;

  if(*pgDir == 0)
    *pgDir = ((dword)newDir()) | PD_FLAGS;
    
  return F_ADDR(*pgDir) + pageind;
}
static void initUniverse() {
  require(&_memory_);

  pageCounters = newDir();

  Dir* pgDir = newDir();
  
  dword i;
  for(i=0;i<DIR_SIZE - 8;i++)
    (*pgDir)[i] = (i << 22) | F_PRESENT | F_RW | F_4M;
  for(;i<DIR_SIZE;i++)
    (*pgDir)[i] = 0;
  
  kernelSpace.pageDir = pgDir;

  setPageDirectory(kernelSpace.pageDir);
  enablePaging();
}
Feature _universe_ = {
  .state = DISABLED,
  .label = "universe",
  .initialize = &initUniverse
};
void mapPage(Universe* univ,dword vpage,void* page,byte rw) {
  Dir* pgDir = univ->pageDir;
  DirEntry* oldmap = dirVal(pgDir,vpage);
  
  if(*oldmap != 0) {
    DirEntry* oldCounter = dirVal(pageCounters,*oldmap);
    
    (*oldCounter)--;
    
    if(IS(*oldCounter,0))
      freePage(F_ADDR(*oldmap));
  }
  
  page = (dword)page & 0xfffff000;

  if(IS(page,NULL))
    *oldmap = 0;
  else {
    DirEntry* counter = dirVal(pageCounters,page);

    *oldmap = (dword)page | PT_FLAGS(rw);
    (*counter)++;
  }
}

