System calls in Omega
=====================

As with most other systems, you may communicate with Omega using system calls. Omega syscalls follow
a simple calling convention similar to other systems :

  1. Load eax with the syscall number   
  2. Load ebc, ecx and edx in that order with the appropriate additional parameters
    (for syscalls with less than three parameters, the unused ones are ignored)
  3. call `int 30h`
  
The current version of Omega provides the following syscalls :

| Arguments to int 30h	    | Syscall name| Description									   |
| ----			    | ----	  | ----									   |
| 0			    | DIE	  | End the current thread.							   |
| (1, univ, neweip)	    | SPARK       | Spark a thread on the given universe at eip=neweip				   |
| (2, vaddr, n) 	    | ALLOC       | Allocates n new pages starting at virtual address vaddr			   |
| (3, sem)		    | ACQUIRE     | Acquire a semaphore or wait.                                		   |
| (4, sem, n)		    | RELEASE     | Release at most n threads waiting on a semaphore. If n==0, release all of them.|
| (5, univ, local, distant) | MAPTO       | Map a page from this to another universe					   |
| (6, univ, distant, local) | MAPFROM     | Map a page from another to this universe					   |
| (7, univ, eip)	    | WARP	  | Warp the current task to another universe at eip				   |
| 8			    | SPAWN       | Spawn a new child universe							   |
| (9,univ)                  | ANIHILATE   | Blast a universe out of existence                                              |
| (10,seconds,millis)       | WAIT        | Wait for some time.                                                            |

Universes
---------

Universes are Omega's version of a process or shared library. They
embody the space component in the space-time continuum that is a
program. Universes provide threads with a virtual memory space that
can be shared in part with other universes for communication or
synchronization.

A universe may spawn a child universe using the SPAWN syscall. The
syscall returns an index that can be used as an index in subsequent
syscalls, such as SPARK, MAPTO, MAPFROM or WARP.

A universe may map a page to or from another universe, where the other
universe is identified by an index, which is interpreted in the
following way :
   
   * for an index of -1, the current universe is affected by the call
   * for an index of -2, it is the universe from the last warp that is affected
   * a non-negative index indicates a child of the current universe, as returned by a previous call to
     SPAWN

The distant and local addresses given to MAPTO and MAPFROM are virtual
addresses in respectively the destination and source universe.

Threads and Semaphores
----------------------

Once universes are spawned and initialized, threads may be sparked at
arbitrary addresses with SPARK. Each thread possesses its own stack space, and
threads can warp between universes using the WARP syscall.

Threads can wait for events from the kernel or other threads using
semaphores. First, a thread decides on an address at which to place
the semaphore. If the chosen address is on shared memory, threads in
other universes will be able to interact with the same
semaphore. 

Then, the thread calls the ACQUIRE syscall, which makes it
wait until the resource is free (non-zero), then restarts it,
returning the last value read. When several threads are restarted
simultaneously, each is returned a unique token which may be used to
distinguish them.

