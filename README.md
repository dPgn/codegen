codegen – a dynamic code generation library

The goal of this project is to be a simple (header only), fast, and portable dynamic code
generation tool for implementing JITs and other dynamic compilers in C++. The
motivation comes from the effort required to use the existing tools. Dynamic
code generation is inherently complicated, enough. The challenge is to not
make it more complicated than it necessarily has to be.

The application can pass the IR to be converted to machine code in textual form, or directly generate it using a sort of a DSL-like API:

    ir::code code;

    auto i32        = code(ir::Int(-32));
    auto funtype    = code(ir::Fun(0, i32, i32, i32));
    auto fun        = code(ir::Enter(funtype));
    auto arg0       = code(ir::Arg(fun, 0));
    auto arg1       = code(ir::Arg(fun, 1));
    auto rval       = code(ir::RVal(fun));
    auto sum        = code(ir::Add(arg0, arg1));

    code(ir::Move(rval, sum));
    code(ir::Exit(fun));

The textual IR is somewhat shorter and more compact (but can actually be more cumbersome when code is generated from traversing an AST, for example):

    std::string codetext =
        "[ fun: Enter [ Fun 0 [ Int -32 ] [ Int -32 ] [ Int -32] ] ]"
        "[ Move [ RVal fun ] [ Add [ Arg fun 0 ] [ Arg fun 1 ] ] ]"
        "[ Exit fun ]";

    ir::code code = textual(codetext).code();

The above examples are copy-pasted from test/public_test.h++.
These cases generate the shortest possible code (5 bytes):

    mov eax, edi    ; 89 F8
    add eax, esi    ; 01 F0
    ret             ; C3

There is now a trivial optimizer in simplify.h++. A simple test case in test/simplify_test.h++ verifies that

    [ fun: Enter [ Fun 0 [ Int -32 ] ] ]
    [ a: Temp [ Int -32 ] ]
    [ b: Temp [ Int -32 ] ]
    [ c: Temp [ Int -32 ] ]
    [ Move a [ Mul [ Cast [ Int -32 ] [ 2 ] ] [ 3 ] ] ]
    [ s0: SkipIf [ Gt a [ 5 ] ] ]
    [ Move b [ 4 ] ]
    [ s1: Skip ]
    [ Here s0 ]
    [ Move b a ]
    [ Here s1 ]
    [ Move c [ Sub [ Cast [ Int -32 ] [ 13 ] ] b ] ]
    [ Move [ RVal fun ] [ Mul b c ] ]
    [ Exit fun ]

becomes

    [ fun: Enter [ Fun 0 [ Int -32 ] ] ]
    [ Move [ RVal fun ] [ Cast [ Int -32 ] [ 42 ] ] ]
    [ Exit fun ]

The code generation process has following steps:

* program creates code in the intermediate representation, either using DSL implemented by ir::code class directly, or in textual form
* intermediate representation is internally rewritten in a form more suitable for optimizations
* parts of the typesystem that aren't natively supported (structures, most importantly) are applied
* basic optimizations (constant propagation, etc.) – and perhaps also something more advanced (loop optimizations, autovectorization), one day
* instruction selection rewrites the IR in its RTL form where most native machine instructions are represented as moves
* register allocation
* stack frame allocation for variables that did not fit in registers
* control flow is rewritten in a form that makes sense for code generation, and other small final adjustments are performed
* target layer generates the native code using the internal "assembler"

A partial list of things to do before codegen can be considered release-ready:

* global symbols and variables as well as a way to call other functions than the one in the beginning of the sole module
* pointers, aggregate types, etc.
* complete calling convention support (caller and callee saved registers, actually calling functions)
* stack frame allocation (running out of registers during register allocation should slow the compiled code down, not crash the compiler)
* floating point and vector instruction support

I have written the x86 architecture specific part so that 32 bit support would be easy to add, so feel free if you think you need it. I won't. :)
I think ARM support will still need to be both 32 bit and 64 bit once I get around to it,
and possibly forever (i.e. next 10 years or so at least) due to the popularity of cheap simple 32-bit ARM microcontrollers in the embedded world.
I want the whole pipleline to run on one architecture first, though, before I implement other targets.

Any comments are appreciated.
