TEST(X86RTL, Basic)
{
    std::string codetext =
        "[ fun: Enter [ Fun 0 [ Int -64 ] [ Int -64 ] [ Int -64] ] ]"
        "[ x: Temp [ Int -64 ] ]"
        "[ y: Temp [ Int -64 ] ]";

    codetext = codetext +
        "[ Move [ Reg x " + std::to_string(ir::x86::id(X)) + " ] [ Arg fun 0 ] ]" +
        "[ Move [ Reg y " + std::to_string(ir::x86::id(Y)) + " ] [ Arg fun 1 ] ]" +
        "[ r: Reg [ RVal fun ] " + std::to_string(ir::x86::id(x86::RAX)) + " ]"
        "[ a: Add x y ]"
        "[ b: Mul x [ 13 ] ]"
        "[ Move r [ Add a b ] ]"
        "[ Exit fun ]";

    ir::code code = textual(codetext).code();
    ir::code rtl = x86::rtl<ir::code, 64>(code);

    ir::code final;

    ra<x86::regs64>().process(final, rtl, 1);

//    std::cout << rtl.text() << std::endl;
//    std::cout << final.text() << std::endl;

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen;
    final.pass(gen);

    ASSERT_EQ(42, gen.fun()(4, -14));
}
