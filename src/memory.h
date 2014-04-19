#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY

#include "constants.h"

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

typedef union page {
  union page* next;
  byte data[PAGE_SIZE];
} page;

#define NULL 0

extern int page_count;

void init_memory();
void* allocate_page();
void free_page(void*);

#endif
