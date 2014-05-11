#include <util/pool.h>
#include <cpu/syscall.h>

void* curPage = (void*)HEAP_START;

static void* allocPage() {
  void* ret = curPage;
  curPage+=PAGE_SIZE;
  syscall_alloc(ret,1);
  return ret;
}

void* poolAllocU(Pool* pool) {
  if(IS(pool->head,0)) {
    /* We allocate a new page for our pool */
    void* newp = allocPage();
    PoolBlock* last = 0;
    int limit = PAGE_SIZE - pool->blockSize;

    int i;
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
void poolFreeU(Pool* pool,void* block) {
  PoolBlock* newh = block;
  newh->next = pool->head;
  pool->head = newh;
}

