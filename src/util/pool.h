#ifndef INCLUDED_POOL
#define INCLUDED_POOL

#include <util/memory.h>

void* poolAlloc(Pool* pool);
void poolFree(Pool* pool,void* p);

#endif
