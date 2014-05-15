#include <constants.h>
#include <util/memory.h>
#include <device/framebuffer.h>

page* page_head;
int page_count;

void* kAllocPage() {
  void* ret = NULL;
  if(page_head != NULL) {
    ret = page_head;
    page_head = page_head->next;
  }

  return ret;
}

void kFreePage(void* _p) {
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
static void initialize() {
  MMapEntry* entries = (void*)MEM_MAP;
  dword size = *(dword*)MEM_MAP_SIZE;
  int i;

  page_count = 0;
  page_head = NULL;
  for(i=0;i<size;i++) {
    page* current = (page*)(entries[i].base_lo & ~(PAGE_SIZE-1));
    dword end = ((dword)current) + entries[i].length_lo;

    if(current < (page*)HEAP_START)  current = (page*)HEAP_START;
    
    while(((dword)current) < end) {
      current->next = page_head;
      page_head = current;
      page_count++;
      current++;
    }
  }

  PAD printf("Detected %d bytes of memory spread across %d pages\n",page_count*PAGE_SIZE,page_count);
}
Feature _memory_ = {
  .state = DISABLED,
  .label = "memory",
  .initialize = &initialize
};

void* kPoolAlloc(Pool* pool) {
  if(IS(pool->head,0)) {
    /* We allocate a new page for our pool */
    void* newp = kAllocPage();
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
void kPoolFree(Pool* pool,void* block) {
  PoolBlock* newh = block;
  newh->next = pool->head;
  pool->head = newh;
}

void memcpy(void* dst_,void *src_,int n) {
  dword *src = src_,*dst = dst_;
  dword* end = src+(n>>2);
  
  while(src != end) {
    *dst = *src; src++; dst++;
  }
  int i;
  for(i=0;i<(n&3);i++)
    ((byte*)dst)[i] = ((byte*)src)[i];
}
int strlen(char* s) {
  char* c = s;
  while(*c != '\0') c++;
  return c-s;
}
