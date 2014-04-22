Omega : A mini-kernel for humble applications
=============================================

Operating systems are considered to be some of the most complex
programs imaginable, since they have to be able to handle every aspect
of the craft, from low-level whackiness to high-level folly.

We perceive this as the truth because the majority of systems today,
mostly represented by Linux, Windows and BSD, are built upon
macrokernels, which do indeed possess vast functionalities. Since all
we see are macrokernels, we have a strong tendency to believe they
are the only approach to kernel development.

Omega is a simple microkernel, which was designed to keep out of the
way and let user programs do their stuff. The only features provided
by the kernel itself are memory allocation, sharing and mapping, and
task scheduling and synchronization. No filesystems, no users, no
permissions, and no devices are handled beyond the absolute necessary.

But what about the security, I hear you cry ? What about
encapsulation, and protection, and quotas and limits ? What about
graphics and networking and sound ? These can all be implemented as
privileged user-space processes without interfering with kernel
code, enabling greater flexibility and simplicity than monolithic
kernels.

The system primitives defined in Omega allow a parent program to
intercept system calls from its children and handle them instead of
the kernel (whithout privilege escalation, it goes without saying).
Omega considers every program to be a virtual machine that can itself
act as a kernel for other programs. 

While this method might sound inefficient, these are not the kinds of
virtual machines where the hardware itself is emulated. They are just
machines (aka. programs) that possess their own virtual environment in
which to execute without disturbing neighbouring programs.

Simply efficient
================

Since its purpose is so focused, the Omega kernel itself can afford to
be quite small (less than 20k at the time of writing), and extremely
efficient since there are no complex data structures to initialize and
maintain or devices to wait upon. It can boot instantly on almost any
(compatible, x86) computer and be up and running in less time than it took
you to read this sentence.

Other components can be loaded and set up just as fast, and I can't
imagine there being more than a few dozens of those : one for
abstracting access to each detected peripheral (so about 20 of those
tops), one to reference them all for easy access, one to spawn and
monitor user programs. Counting large, that leaves us with less than
thirty additional modules to be loaded before the operating system may
be used fully. By my calculations, if each module takes as long as the
kernel to load and initialize (~10ms), a fully-functional system is a
few milliseconds away from pushing a button, which is fast enough for
my own use.

Compiling and running Omega
===========================

Compiling Omega is dead simple, just run `cook` from the root of the
source directory and grab the boot image `dist/Omega.img` that was
generated.

I run my own tests with Bochs, an open-source emulator, so Omega is
guaranteed to work on that. Technically, it should work on any
x86-capable machine, since it was developped from the Intel
specification, but I haven't tested it on real hardware yet.

Running Omega in Bochs is really easy too : just tell bochs to treat
Omega.img as a primary drive and boot from that drive. A configuration
file is provided in `bochs/bochsrc` as an example, although you may need
to adjust the paths to get it working.

Omega is open-source software provided under the Free Beer Public
License, which makes it available free of charge as well as of
spirit. You may thus use or modify it in part or in full to fit your
needs, as long as you also make your work free of charge.

Remaining work
==============

Omega is not still finished yet, as some features announced above
have not yet been implemented. They will be in the coming times, but for
now they are listed here : 

  * Scheduling tasks (I still have to figure out this TSS thing)
  * syscalls

