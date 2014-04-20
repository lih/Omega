#include "constants.h"
#include "memory.h"

page* page_head;
int page_count;

void* allocate_page() {
  void* ret;
  if(page_head != NULL) {
    ret = page_head;
    page_head = page_head->next;
  }
  return ret;
}

void free_page(void* _p) {
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

void init_memory() {
  MMapEntry* entries = (void*)MEM_MAP;
  dword size = *(dword*)MEM_MAP_SIZE;
  int i;

  page_head = NULL;
  for(i=0;i<size;i++) {
    page* current = (page*)(entries[i].base_lo & ~(PAGE_SIZE-1));
    dword end = ((dword)current) + entries[i].length_lo;

    if(current < 0x100000)  current = 0x100000;
    
    while(((dword)current) < end) {
      current->next = page_head;
      page_head = current;
      page_count++;
      current ++;
    }
  }
}
