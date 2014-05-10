#ifndef INCLUDED_POOL
#define INCLUDED_POOL

#include <cpu/memory.h>

void* poolAllocU(Pool* pool);
void poolFreeU(Pool* pool,void* p);

#endif
