The MeXa scripting language
===========================

After implementing the basic OS functionality (threads, syscalls and
memory management), I realized that Omega needed something to make it
more interactive (as changing kernel code whenever I need to do
something is a shady practice at best). I don't have the requisite
architecture and abstractions needed to have a visual interface just
yet, so I had to settle for a textual language for now. Once I write a
working GUI, MeXa shall be adapted to a more visual form.

To meet my monthly quota of obtrusiveness, I composed a short and
semi-formal definition of MeXa :

> MeXa is a lazy, reactive, dynamically typed language with a Lisp-like
> syntax plus a few sugary extensions.

It's good to meet your own expectations. 

Now that this is out of the way, we can start looking at what the
language can actually does, and how it does it.

MeXa philosophy
---------------

The MeXa language is based on the premise that your system is an
elaborate machine (which it is) composed of gears (which is a small
stretch) that possess cogs, or dependencies, to other gears, or
expressions, in order to yield a torque, or value.

In a physical machine, torque is usually expressed in the unit of a
3-dimensional transformation, such as rotation or translation. In a
virtual machine, torque can be anything we want it to be, such as 
a number, some text or a dictionary (more on that later). 

Since a computer is just a fancy calculator, yielding torque is usually
expressed by calculating a value (and showing it on the screen) instead of
powering a car, but the principle is the same.

As a computer owner, you may want to add gears that fit your own
needs, and mesh them to the existing gears. Additionally, since we
make mistakes all the time, it seems reasonable to also allow
ourselves to replace gears sometimes, throwing the old ones away.

That's all, folks ! Now let's take a closer look at the language...

MeXa basics
-----------

In MeXa, like in many other languages, you can set and get
variables. Variables are the way to access gears (by naming them), in
order to be able to replace them later.

Some variable setting in MeXa :

    | x = 4
    = 4
    | y = 6
    = 6
    | z = (+ x y)
    = 10

Notice the `(+ x y)` form in the last expression. That is what
"Lisp-like syntax" means.  It's essentially the same as `x+y`, with
the `+` in front. Combinations of any number of gears can be created
this way, so that if I want to add, say, five elements, all I have to do
is say `(+ x y z a b)` and be done with it.

What is a combination ? It means any gear of shape `(combinator gear1
gear2...)`. In the previous example, our combinator is `+`, which adds
up the torques of its gears, which must be of a numeric unit.

Many "real-world" combinators like `+` or `*` are unary or binary, so
there is another way of expressing those specific combinations. You
may write `a:+ b` instead of `(+ a b)` and `x.f` instead of `(f x)` in
any gear to alleviate some of the burden of Lisp syntax in
mathematical expressions.

Those operators associate to the left interchangeably, so that `a:+ b:* c.sqrt`
is read as `(sqrt (* (+ a b) c))`.

To recapitulate, the affectation of `z` could have been carried out like this instead :

    | z = x:+ y
    = 10

There are other basic units as well :

  * arrays: `[x y z...]`
  * text: `"hello"`
  * combinator: `+`, `if`, `@`
  
What if we suddenly decide that `4` is *definitely* not a proper torque for
`x` and want to change it to a better one, say `"four"` ? We tell it like so :

    | x = "four"
    = "four"
    | x 
    = "four"

And voilà ! `x` has a better torque now ! Everything is well again,
but now `z` doesn't make sense, since now it's torque is `"four":+ y`
(which I'm sorry to say doesn't yield `"ten"`, that would almost be
too awesome). Let's check it out :

    | z
    = 6

What happened there is that `x` has been ignored, as it was not a number,
and `z` became the sum of `6`, which is `6`.

What also happened is that the new torque that we assigned to `x` was 
magically propagated to yield a new torque for `z`.

If you think of MeXa expressions as equations, that means that the
system is enforcing the equations to be true no matter what gears you
replace.

The same result could have been achieved by writing 

    | z = x:+ y
    = 0
    | x = 4
    = 4
    | y = 6
    = 6
    | z
    = 10

