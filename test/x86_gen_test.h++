TEST(X86Gen, Minimal)
{
    ir::code code;

    code(ir::Move(ir::x86::id(x86::RAX), code(42)));
    code(ir::Ret());

    x86::function_gen<std::int64_t()> gen;
    code.pass(gen);
    ASSERT_EQ(42, gen.fun()());
}
