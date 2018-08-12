TEST(X86Gen, Minimal)
{
    ir::code code;

    code(ir::Move(ir::x86::id(x86::RAX), code(42)));
    code(ir::Ret());

    x86::function_gen<std::int64_t()> gen;
    code.pass(gen);
    ASSERT_EQ(42, gen.fun()());
}

TEST(X86Gen, BasicInstructions)
{
    ir::code code;

    // All instructions that produce a result in the architecture level IR are moves. The operands
    // of the move encode the address mode and function of the instruction. The move itself will
    // also be encoded on two-address architectures.
    code(ir::Move(ir::x86::id(x86::RAX), code(ir::Add(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen;
    code.pass(gen);
    ASSERT_EQ(42, gen.fun()(15, 27));
}
