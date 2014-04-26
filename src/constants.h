#ifndef INCLUDED_CONSTANTS
# define INCLUDED_CONSTANTS

#define KERNEL       0x1200
#define MEM_MAP      0x1008
#define MEM_MAP_SIZE 0x1000

#define KERNEL_STACK 0x10000
#define INT_STACK    0x11000

#define SMAP_MAGIC   0x534D4150
#define PAGE_SIZE    0x1000
#define FB_MEM       0xB8000
#define FB_END       0xB8f9f

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10

extern void kernel_end;
extern int ksize;

#endif
