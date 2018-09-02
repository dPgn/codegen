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

The above examples are copy-pasted from test/public_test.h++. These cases pass, but are extremely simple.

Once complete, the code generation process has following steps:

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

* simplification (optimization step that combines constant propagation, dead code elimination, and trivial common subexpression elimination)
* completion of the register allocator – it currently does little more than gets the simple tests to pass
* global symbols and variables as well as a way to call other functions than the one in the beginning of the sole module
* pointers, aggregate types, etc.
* complete calling convention support (caller and callee saved registers, actually calling functions)
* stack frame allocation (running out of registers during register allocation should slow the compiled code down, not crash the compiler)
* floating point and vector instruction support

Is there any need to support 32 bit x86 still in late 2018? Is someone somewhere still running a 32 bit OS on Intel/AMD?
I have written the architecture specific part so that 32 bit support would be easy to add, but is it worth the trouble?
I think AMD support will still need to be both 32 bit and 64 bit once I get around to it.
I want the whole pipleline to run on one architecture first, though, before I implement other targets.

Any comments are appreciated.
