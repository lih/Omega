#ifndef INCLUDED_POOL
#define INCLUDED_POOL

#include <util/memory.h>

void* poolAllocU(Pool* pool);
void poolFreeU(Pool* pool,void* p);

#endif
