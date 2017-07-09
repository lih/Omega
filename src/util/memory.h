#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY

#include <constants.h>
#include <util/feature.h>

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
#define SZ(a) (sizeof(a)/sizeof((a)[0]))
#define AFTER(e) (((void*)(e))+sizeof(*(e)))

extern Feature _memory_;
extern int page_count;
extern int alloc_count;

void* kAllocPage();
void kFreePage(void*);

void memcpy(void* dst, void* src, int n);
int strlen(char* s);

/* 
   Allocation pools of fixed-size objects (with constant-time
   alloc() and free() operations).
*/
typedef union PoolBlock {
  union PoolBlock* next;
  byte data[0];
} PoolBlock;

typedef struct {
  PoolBlock* head;
  int blockSize;
} Pool;

void* kPoolAlloc(Pool* pool);
void kPoolFree(Pool* pool,void* p);

#endif
