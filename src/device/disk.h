#ifndef INCLUDED_DISK
#define INCLUDED_DISK

#include <cpu/memory.h>

void ata_read(word* dest,dword nb_words,dword lba);

#endif
