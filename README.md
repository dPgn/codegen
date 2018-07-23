codegen – a dynamic code generation library

This is the first public, and hopefully final, iteration of my dynamic code
generation library.

The goal of this project is to be a simple, fast, and portable dynamic code
generation tool for implementing JITs and other dynamic compilers. The
motivation comes from the effort required to use the existing tools. Dynamic
code generation is inherently complicated, enough. The challenge is not to
make it more complicated than it necessarily has to be.

There are great tools, such as LLVM, if you need extreme performance and
flexibility. However, very often the order-of-magnitude speed penalty of
interpreted versus dynamically compiled code is our only concern, and the last
10% or so that we could theoretically squeeze out of the execution time of the
generated code justifies neither the effort required to use such a high-end
compiler backend nor the slow execution of the backend itself.

For this iteration I chose to use Google Test for the first time (ever, for any
project, so I'm probably doing something 'wrong'). Also, I decided to take a
strict test driven bottom-up approach, as I find it most suitable for a project
like this.

In my previous iterations I generated code directly from my intermediate
representation. This made the code too difficult to read and maintain, so I
decided to add an assembly layer. It's obvious, when you think about it:
writing code is much easier than writing code that writes code. If I would
rather write code in assembly than in hexadecimal, then I most certainly should
make my code that writes code do it in assembly rather than in hexadecimal.

The planned code generation pipeline looks like this:

* program creates code in the intermediate representation
* basic optimizations are performed (constant propagation, etc.)
* code is rewritten in a subset of IR supported by the target layer
* register allocation
* target layer generates the native code using the internal "assembler"

I am planning on making daily commits for the next few weeks until the library
becomes useful for its intended purpose.

Any comments are appreciated.
