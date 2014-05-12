#include <util/array.h>
#include <util/pool.h>
#include <core/syscall.h>

Pool smallPools[] = {
  { 0 , 4 },
  { 0 , 8 },
  { 0 , 16 },
  { 0 , 32 },
  { 0 , 64 },
  { 0 , 128 },
  { 0 , 256 },
  { 0 , 512 },
  { 0 , 1024 },
  { 0 , 2048 },
  { 0 , 4096 },
};

/* Allocates an array of size up to 1page */
void* newArray(int size) {
  int i = 0;
  int total = size + sizeof(int);
  while((1<<(i+2))<total) i++;
  int* ret = poolAllocU(&smallPools[i]);
  *ret = total;
  return ret+1;
}
void freeArray(void* arr) {
  int* size = arr - sizeof(int);
  int i=0;
  while((1<<(i+2))<*size) i++;
  poolFreeU(&smallPools[i],size);  
}
