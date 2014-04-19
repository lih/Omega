#include "constants.h"
#include "memory.h"

extern page* page_head;

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

