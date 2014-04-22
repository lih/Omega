#include "constants.h"
#include "memory.h"

page* page_head;
int page_count;

void* allocatePage() {
  void* ret;
  if(page_head != NULL) {
    ret = page_head;
    page_head = page_head->next;
  }

  return ret;
}

void freePage(void* _p) {
  page* old = page_head;
  page_head = _p;
  page_head->next = old;
}

typedef struct {
  dword base_lo;
  dword base_hi;
  dword length_lo;
  dword length_hi;
  dword type;
  dword attributes;
} MMapEntry;

void initMemory() {
  MMapEntry* entries = (void*)MEM_MAP;
  dword size = *(dword*)MEM_MAP_SIZE;
  int i;

  page_head = NULL;
  for(i=0;i<size;i++) {
    page* current = (page*)(entries[i].base_lo & ~(PAGE_SIZE-1));
    dword end = ((dword)current) + entries[i].length_lo;

    if(current < 0x10000)  current = 0x10000;
    
    while(((dword)current) < end) {
      current->next = page_head;
      page_head = current;
      page_count++;
      current++;
    }
  }
}

Pool pool(int blocksize) {
  Pool ret = {
    .head = NULL,
    .blockSize = blocksize
  };
  return ret;
}

void* poolAlloc(Pool* pool) {
  if(IS(pool->head,0)) {
    /* We allocate a new page for our pool */
    void* newp = allocatePage();
    PoolBlock* last = 0;
    int i;
    int limit = PAGE_SIZE - pool->blockSize;

    for(i=0;i<=limit;i+=pool->blockSize) {
      PoolBlock* cur = newp + i;
      cur->next = last;
      last = cur;
    }

    pool->head = last;
  }

  void* ret = pool->head->data;
  pool->head = pool->head->next;
  
  return ret;
}
void poolFree(Pool* pool,void* block) {
  PoolBlock* newh = block;
  newh->next = pool->head;
  pool->head = newh;
}
