/*
    codegen – a dynamic code generation library

    Copyright 2018 Oskari Teirilä

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
    // also be encoded on two-address architectures, unless its destination is the same as the
    // first operand of its source node. This, for example, translates into
    // MOV RAX, X
    // ADD RAX, Y
    // However, were the destination of the move X, we would simply have
    // ADD X, Y
    code(ir::Move(ir::x86::id(x86::RAX), code(ir::Add(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen_add;
    code.pass(gen_add);
    ASSERT_EQ(42, gen_add.fun()(15, 27));

    code.clear();

    code(ir::Move(ir::x86::id(x86::RAX), code(ir::Sub(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen_sub;
    code.pass(gen_sub);
    ASSERT_EQ(42, gen_sub.fun()(55, 13));

    code.clear();

    code(ir::Move(ir::x86::id(x86::RAX), code(ir::Mul(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen_mul;
    code.pass(gen_mul);
    ASSERT_EQ(42, gen_mul.fun()(6, 7));

    code.clear();

    code(ir::Move(ir::x86::id(x86::RAX), code(ir::Xor(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen_xor;
    code.pass(gen_xor);
    ASSERT_EQ(42, gen_xor.fun()(0x25, 0xf));

    code.clear();

    code(ir::Move(ir::x86::id(x86::RAX), code(ir::And(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen_and;
    code.pass(gen_and);
    ASSERT_EQ(42, gen_and.fun()(0xfa, 0x2f));

    code.clear();

    code(ir::Move(ir::x86::id(x86::RAX), code(ir::Or(ir::x86::id(X), ir::x86::id(Y)))));
    code(ir::Ret());

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen_or;
    code.pass(gen_or);
    ASSERT_EQ(42, gen_or.fun()(0xa, 0x22));
}
