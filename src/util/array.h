#ifndef INCLUDED_ARRAY
#define INCLUDED_ARRAY

void* newArray(int size);
void freeArray(void* arr);

#define DOTIMES(v,n) int v; for(v=0;v<(n);v++)

#endif
