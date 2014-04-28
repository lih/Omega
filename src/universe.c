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
#define PT_FLAGS (F_PRESENT | F_RW | F_USER)

Dir* pageCounters;
Universe kernelSpace;
Pool univPool;

Dir* newDir() {
  Dir* ret = allocatePage();
  int i;
  for(i=0;i<DIR_SIZE;i++)
    (*ret)[i] = NULL;
  return ret;
}
Universe* newUniverse() {
  require(MEMORY);
  
  Universe* ret = poolAlloc(&univPool);
  ret->pageDir = newDir();
  
  return ret;
}

DirEntry* dirVal(Dir* dir,dword vpage) {
  word dirind = vpage >> 22;
  word pageind = (vpage >> 12) & 0x3ff;
  DirEntry* pgDir = (*dir) + dirind;

  if(*pgDir == 0) {
    DirEntry* newTbl = allocatePage();
    int i;
    for(i=0;i<DIR_SIZE;i++)
      newTbl[i] = 0;
    
    *pgDir = ((dword)newTbl) | PD_FLAGS;
  }
  
  return F_ADDR(*pgDir) + pageind;
}
void initUniverse() {
  require(MEMORY);

  univPool = pool(sizeof(Universe));
  pageCounters = newDir();

  Dir* pgDir = newDir();
  
  dword i;
  for(i=0;i<DIR_SIZE;i++)
    (*pgDir)[i] = (i << 22) | F_PRESENT | F_RW | F_4M;
  
  kernelSpace.pageDir = pgDir;

  setPageDirectory(kernelSpace.pageDir);
  enablePaging();
}
void mapPage(Universe* univ,dword vpage,void* page) {
  Dir* pgDir = univ->pageDir;
  DirEntry* oldmap = dirVal(pgDir,vpage);
  
  printf("mapPage: oldmap=%d page=%x\n",*oldmap,page);
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

    *oldmap = (dword)page | PT_FLAGS;
    (*counter)++;
  }
  printf("mapPage2: oldmap=%x page=%x\n",*oldmap,page);
}
