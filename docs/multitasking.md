Multitasking on Intel x86 architecture
======================================

The hardest part of writing a kernel is correct handling or parallel
and concurrent application. The first step towards that is a working
implementation of task-switching for your brand of processor.

On Intel x86, in protected (32-bit) mode, task switching is handled
via a Task Register containing a selector in the GDT describing a
Task-State Segment (aka. TSS), which is used to store the state of the
task when switching to another (registers, permissions, etc...).
These structures are all described in the Intel Architecture
Reference, Volume 3a, if you need greater detail.

Multitasking and Paging
-----------------------

I came into trouble trying to switch tasks while paging was
enabled. Once the first switch to a user process was made, the kernel could
never be reached again and the processor would hang. 

What I didn't understand at the time was that the current task needed
to be able to write to its own TSS (the page containing the TSS should
be present and writable), AND read from the TSS of kernel tasks (for
which the pages should be present and read-only) for IRQs, exceptions and
other interrupts to be properly handled.

If the TSSs of kernel tasks are not readable from userspace tasks, the
processor will page fault when switching to the schedule handler, then
page fault again when switching to the page fault handler, and so on...

So, for proper handling of multitasking with paging enabled, I
recommend the following :

  * Identity-map the kernel pages that contain the TSSs for all kernel
    tasks (I tend to put the TSS at the top of a task's stack, so that
    means mapping the first stack page). These should be mapped
    read-only so that user processes may not tamper with interrupt
    handlers (like redirecting a syscall to userspace, that kind of
    nasty stuff). That make the kernel stacks visible to all, but all
    the sensitive information is contained elsewhere in the kernel
    (all the user programs might guess would be return addresses in
    kernel code and local values of esp).

  * Map each task's stack (and TSS) at the same address, even when
    switching address-spaces. This would involve reserving some space
    in linear memory for the maximum number of tasks (a thousand or
    two would be a reasonable maximum for hardware threads per
    processors), and assigning a slot to every task, and a stack to
    every slot.
