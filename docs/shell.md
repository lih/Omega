The Omega Shell language
========================

After implementing the basic OS functionality (threads, syscalls and
memory management), I realized that Omega needed something to make it
more interactive (as changing kernel code whenever I need to do
something is a shady practice at best). I don't have the requisite
architecture and abstractions needed to have a visual interface just
yet, so I had to settle for a textual language for now. Once I write a
working GUI, the language shall be adapted to a more visual form.

Omega Shell (I have yet to come up with a good name) is a lazy, reactive,
dynamically typed language with a Lisp-like syntax that makes it easy to parse
and evaluate.

Here is an example of Shell code :

    mode = (if VGA_ON "vga" "framebuffer")

While not impressive by any standard, the following actually defines a
dataflow evaluation pipeline involving the variables `if` and `VGA_ON`, and
the constants `"vga"` and `"framebuffer"`.

In the Shell, every expression is either a thunk in need of forcing,
or a pure value. Evaluation follows the same semantics as it does in
other lazy languages : if a value is pure, return it; otherwise, force
it (which may force other values in the process), then return it.

Being a scripting language and all, the Shell cannot afford to have
pure semantics, since we normally expect the result of our computation
to be produced right now. Instead, the Shell has dynamic semantics,
which allow for thunks to be invalidated when their dependencies
change, thus reevaluating the parts that depend on them. As a concrete
example, here is an interactive excerpt of Shell output :

    > VGA_ON = 1
    > mode
    "vga"
    > VGA_ON = nil
    > mode
    "framebuffer"

As you can see, the `mode` variable gets a new value every time we assign
one to `VGA_ON`.
