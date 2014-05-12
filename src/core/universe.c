#include <constants.h>
#include <util/memory.h>
#include <core/universe.h>
#include <core/schedule.h>
#include <device/framebuffer.h>
#include <core/syscall.h>
#include <cpu/pervasives.h>

#define F_PRESENT 1
#define F_RW      2
#define F_USER    4
#define F_PWT     8 		/* Page write-through */
#define F_CD      16 		/* Cache disabled */
#define F_ACCESSED 32
#define F_4M      128 		/* 4M pages instead of 4k */
#define F_GLOBAL  256

#define PD_FLAGS (F_PRESENT | F_RW | F_USER)
#define PT_FLAGS(rw) (F_PRESENT | ((rw)? F_RW:0) | F_USER)

Dir* pageCounters;
Universe kernelSpace = {
  .dpl = 0,
  .up = 0,
  .left = &kernelSpace,
  .right = &kernelSpace
};
Pool univPool = { NULL, sizeof(Universe) };

Dir* newDir() {
  Dir* ret = allocatePage();
  int i;
  for(i=0;i<DIR_SIZE;i++)
    (*ret)[i] = NULL;
  return ret;
}
void freeDir(Dir* d) {
  int i;
  for(i=0;i<DIR_SIZE;i++) {
    void* pg = F_ADDR((*d)[i]);
    if(pg != 0) freePage(pg);
  }
  freePage(*d);
}

Universe* newUniverse(Universe* father) {
  require(&_memory_);
  
  Universe* ret = poolAlloc(&univPool);
  SET_STRUCT(Universe,*ret,{
      .pageDir = newDir(), 
	.dpl = 0,
	.index = father->down->index,
	.up = father,
	.left = father->down->left, .right = father->down,
	.down = poolAlloc(&univPool)
	});
  father->down->index++;

  ret->left->right = ret;
  ret->right->left = ret;

  ret->down->right = ret->down;
  ret->down->left = ret->down;
  ret->down->index = 0;

  return ret;
}
void freeUniverse(Universe* u) {
  Universe* fence = u->down;
  while(fence->right != fence)
    freeUniverse(fence->right);
  poolFree(&univPool,fence);

  Task* roots[3] = { &activeRoot, &pendingRoot, &waitingRoot };
  int i;
  for(i=0;i<3;i++) {
    Task* cur = roots[i]->next;
    Task* next;
    while(cur != roots[i]) {
      next = cur->next;
      if(cur->univ == u) 
	sys_die(cur);
      cur = next;
    }
  }

  freeDir(u->pageDir);
  poolFree(&univPool,u);
}

DirEntry* dirVal(Dir* dir,void* _vpage) {
#define vpage ((dword)_vpage)
  word dirind = vpage >> 22;
  word pageind = (vpage >> 12) & 0x3ff;
  DirEntry* pgDir = (*dir) + dirind;

  if(*pgDir == 0)
    *pgDir = ((dword)newDir()) | PD_FLAGS;
    
  return F_ADDR(*pgDir) + pageind;
#undef vpage
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
  kernelSpace.down = poolAlloc(&univPool);
  kernelSpace.down->index = 0;
  kernelSpace.down->right = kernelSpace.down;
  kernelSpace.down->left = kernelSpace.down;
  
  setPageDirectory(kernelSpace.pageDir);
  enablePaging();
}
Feature _universe_ = {
  .state = DISABLED,
  .label = "universe",
  .initialize = &initUniverse
};
void mapPage(Universe* univ,void* vpage,void* page,byte rw) {
  Dir* pgDir = univ->pageDir;
  DirEntry* oldmap = dirVal(pgDir,vpage);
  
  if(*oldmap != 0) {
    DirEntry* oldCounter = dirVal(pageCounters,(void*)*oldmap);
    
    (*oldCounter)--;
    
    if(IS(*oldCounter,0))
      freePage(F_ADDR(*oldmap));
  }
  
  page = (void*)((dword)page & 0xfffff000);

  if(IS(page,NULL))
    *oldmap = 0;
  else {
    DirEntry* counter = dirVal(pageCounters,page);

    *oldmap = (dword)page | PT_FLAGS(rw);
    (*counter)++;
  }
}

