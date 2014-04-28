#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY

#include "constants.h"
#include "feature.h"

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

typedef union page {
  union page* next;
  byte data[PAGE_SIZE];
} page;

#define NULL 0
#define PACKED __attribute__((packed))
#define IS(a,b) ((a)==(b))
#define SET_STRUCT(t,v,...) { t __tmp__ = __VA_ARGS__; v = __tmp__; }
#define AS(t,e) (*(t*)(&(e)))

extern Feature _memory_;
extern int page_count;

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
