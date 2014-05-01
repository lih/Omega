System calls in Omega
=====================

As with most other systems, you may communicate with Omega using system calls. Omega syscalls follow
a simple calling convention similar to other systems :

  1. Load eax with the syscall number   
  2. Load ebc, ecx and edx in that order with the appropriate additional parameters
    (for syscalls with less than three parameters, the unused ones are ignored)
  3. call `int 30h`
  
The Omega syscalls are as follows :

  * eax=0 (DIE) : ends the current thread.
  * eax=1, ebx=univ, ecx=neweip (SPAWN) : spawns a thread on the given universe
    at eip=neweip
  * eax=2, ebx=vaddr (ALLOC): allocates a new page at virtual address vaddr in the
    current task's universe  