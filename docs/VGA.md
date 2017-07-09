The Video Graphic Array (VGA)
=============================

An important aspect of a working OS is the handling of the machine's
input and output to other peripherals, such as the screen or keyboard.
A simple VGA driver is the first step towards a full-blow window
manager.

From the CPU's point of view, the only way to communicate with the
outside world is through data ports that may be connected to some pins
on an external device. On the Intel x86 architectures, there are 65536
such I/O ports indexed by a 2-byte port address.

Each device may use its own I/O ports, and follow its own conventions
about accessing those ports. The most difficult part of writing a
device driver (in fact, the *only* part of writing a driver) is to
find out on what ports this device listens and what shape of data can
be written to and retrieved from them.

The VGA specification is a standard followed by most video cards that
is easy to understand, well-defined and simple to write for. As far as
I can gather, there are some VGA registers that dictate the behaviour
of the graphics card. These registers can be read and set using pairs
of 1-byte I/O ports, one index port and one data port. The index port
selects the byte that will be accessed within the register, starting
at 0 for the first byte. Once the index port is set to the proper
value, the data port can be read or written, which will change the
value of the register on the video card.

There are two important registers on a VGA card, the Graphics Register and
the Sequencer Register.

| Register | Index Port | Data Port |
|-------  |---------  |--------- |
| Graphics Register | 3ceh | 3cfh |
| Sequencer Register | 3c4h | 3c5h |

