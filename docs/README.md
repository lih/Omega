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
privileged user-space processes without interfering with kernel code,
enabling greater flexibility and simplicity than monolithic kernels
who need to carry all that extra complexity.

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
----------------

Since its purpose is so focused, the Omega kernel itself can afford to
be quite small (less than 30k at the time of writing this), and
extremely efficient since there are no complex data structures to
initialize and maintain or devices to wait upon. It can boot instantly
on almost any (compatible, x86) computer and be up and running in less
time than it took you to read this sentence.

Other components can be loaded and set up just as fast, and I can't
imagine there being more than a few dozens of those : one for
abstracting access to each detected peripheral (so about 20 of those
tops), one to reference them all for easy access, one to spawn and
monitor user programs. Counting large, that leaves us with less than
thirty additional modules to be loaded before the operating system may
be used fully. By my calculations, if each module takes as long as the
kernel to load and initialize (<10ms), a fully-functional system is a
few milliseconds away from pushing a button, which I deem *fast enough*(tm)

Compiling and running Omega
===========================

Omega is written in C and assembly, so very little is actually needed
to compile it. I use `gcc` and `nasm`, which work just fine. I use
`cook` as my build tool, but it should be simple enough to write a
Makefile with the same functionality (actually, `cook` can do that for
you).

Once all the tools are installed, compiling Omega is dead simple, just
run `cook` from the root of the source directory and grab the boot
image `dist/Omega.img` that was generated.

I run my own tests with Bochs, an open-source emulator, so Omega is
guaranteed to work on that. Technically, it should work on any
x86-capable machine, since it was developped from the Intel
specification, but I haven't tested it on real hardware yet (I like to
think inside the Bochs ;-) )

Running Omega in Bochs is really easy too : just tell Bochs to treat
`Omega.img` as a primary drive and boot from that drive. Configuration files
are provided in `run/fast` and `run/gui` to run Omega on either a regular (fast)
or a debugging (gui) interface. 

Remaining work
==============

Omega is not still finished yet, as some features announced above
have not yet been implemented. They will be in the coming times, but for
now they are listed here : 

  * Perhaps a small filesystem driver to bootstrap the first few universes
  * Perhaps even a small TFTP and NIC driver for booting over PXE

...aaand we'll be done ! All subsequent work shall be directed towards
the user interface, involving sound, video, keyboard and mouse, disk
and network interfaces that will allow programs to easily access most
aspect of a typical computer.

In order from most to least important, I plan to implement the
following functionalities :
 
  * A general filesystem-environment-discovery interface, associating
    data to keys on a system-wide scale. It may be used to access
    files, permissions, virtual filesystem, environment variables and
    many other things, I'm sure.
  * A graphical interface stack (from driver to windowing environment)
    and simple toolkit.
  * A general-purpose editing program to manipulate the abstract
    concepts in an intuitive way.
  * A programming language to take advantage of the editor, and yield
    the best productivity from Omega. A compiler to turn that language
    into reality.
  * Any other tool, game or program I might think of :-D
