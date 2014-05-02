#ifndef INCLUDED_CONSTANTS
# define INCLUDED_CONSTANTS

#define KERNEL_START 0x1200
#define MEM_MAP      0x1008
#define MEM_MAP_SIZE 0x1000

#define KERNEL_STACK 0x10e00
#define INT_STACK    0x12000
#define SYS_STACK    0x13000
#define KEY_STACK    0x14000
#define EXC_STACK    0x15000
#define IRQ_STACK    0x16000
#define HEAP_START   0x16000

#define SMAP_MAGIC   0x534D4150
#define PAGE_SIZE    0x1000
#define FB_MEM       0xB8000
#define FB_END       0xB8f9f

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10

#define TIMER_FREQ 256

#define SYS_DIE 0
#define SYS_SPARK 1
#define SYS_ALLOC 2
#define SYS_ACQUIRE 3
#define SYS_RELEASE 4
#define SYS_MAPTO 5
#define SYS_MAPFROM 6
#define SYS_WARP 7
#define SYS_SPAWN 8
#define SYS_ANIHILATE 9
#define SYS_WAIT 10
#define SYS_COUNT 11

extern void KERNEL_END;
extern int const KERNEL_SIZE;

#endif
