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
#define IS(a,b) ((a)==(b))

extern int page_count;

void initMemory();

void* allocatePage();
void freePage(void*);

typedef union PoolBlock {
  union PoolBlock* next;
  byte data[0];
} PoolBlock;

typedef struct {
  PoolBlock* head;
  int blockSize;
} Pool;

Pool pool(int blockSize);
void* poolAlloc(Pool* pool);
void poolFree(Pool* pool,void* p);

#endif
