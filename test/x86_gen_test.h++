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

    auto fun = code(ir::Enter(code(ir::Fun(0, code(ir::Int(-64))))));
    auto rval = code(ir::RVal(fun));

    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::RAX))), code(42)));
    code(ir::Exit(fun));

    x86::function_gen<std::int64_t()> gen;
    code.pass(gen);
    ASSERT_EQ(42, gen.fun()());
}

template<class NODE> std::int64_t test_basic(std::int64_t x, std::int64_t y)
{
    ir::code code;

    auto i64 = code(ir::Int(-64));
    auto fun = code(ir::Enter(code(ir::Fun(0, i64, i64, i64))));
    auto rval = code(ir::RVal(fun));
    auto argx = code(ir::Arg(fun, 0));
    auto argy = code(ir::Arg(fun, 1));

    // All instructions that produce a result in the architecture level IR are moves. The operands
    // of the move encode the address mode and function of the instruction. The move itself will
    // also be encoded on two-address architectures, unless its destination is the same as the
    // first operand of its source node. This, for example, translates into
    // MOV RAX, X
    // INSTR RAX, Y
    // However, were the destination of the move X, we would simply have
    // INSTR X, Y
    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::RAX))), code(NODE(code(ir::Reg(argx, ir::x86::id(X))), code(ir::Reg(argy, ir::x86::id(Y)))))));
    code(ir::Exit(fun));

    x86::function_gen<std::int64_t(std::int64_t, std::int64_t)> gen;
    code.pass(gen);

    return gen.fun()(x, y);
}

TEST(X86Gen, BasicInstructions)
{
    ASSERT_EQ(42, test_basic<ir::Add>(15, 27));
    ASSERT_EQ(42, test_basic<ir::Sub>(55, 13));
    ASSERT_EQ(42, test_basic<ir::Mul>(6, 7));
    ASSERT_EQ(42, test_basic<ir::Xor>(0x25, 0xf));
    ASSERT_EQ(42, test_basic<ir::And>(0xfa, 0x2f));
    ASSERT_EQ(42, test_basic<ir::Or>(0xa, 0x22));
}

template<class NODE, bool sgnd = true> bool test_compare(std::int64_t x, std::int64_t y)
{
    ir::code code;

    auto i64 = code(ir::Int(sgnd? -64 : 64)), b = code(ir::Int(0));
    auto fun = code(ir::Enter(code(ir::Fun(0, b, i64, i64))));
    auto rval = code(ir::RVal(fun));
    auto argx = code(ir::Arg(fun, 0));
    auto argy = code(ir::Arg(fun, 1));

    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::RAX))), code(NODE(code(ir::Reg(argx, ir::x86::id(X))), code(ir::Reg(argy, ir::x86::id(Y)))))));
    code(ir::Exit(fun));

    x86::function_gen<bool(std::int64_t, std::int64_t)> gen;
    code.pass(gen);
    return gen.fun()(x, y);
}

TEST(X86Gen, Compare)
{
    ASSERT_FALSE(test_compare<ir::Eq>(12345, 123));
    ASSERT_TRUE(test_compare<ir::Neq>(12345, 123));
    ASSERT_FALSE(test_compare<ir::Lt>(12345, 123));
    ASSERT_FALSE(test_compare<ir::Lte>(12345, 123));
    ASSERT_TRUE(test_compare<ir::Gt>(12345, 123));
    ASSERT_TRUE(test_compare<ir::Gte>(12345, 123));

    ASSERT_FALSE(test_compare<ir::Eq>(13, 666));
    ASSERT_TRUE(test_compare<ir::Neq>(13, 666));
    ASSERT_TRUE(test_compare<ir::Lt>(13, 666));
    ASSERT_TRUE(test_compare<ir::Lte>(13, 666));
    ASSERT_FALSE(test_compare<ir::Gt>(13, 666));
    ASSERT_FALSE(test_compare<ir::Gte>(13, 666));

    // Unsigned versions should behave as if -1 > 13
    ASSERT_TRUE((test_compare<ir::Lt, false>(13, -1)));
    ASSERT_TRUE((test_compare<ir::Lte, false>(13, -1)));
    ASSERT_FALSE((test_compare<ir::Gt, false>(13, -1)));
    ASSERT_FALSE((test_compare<ir::Gte, false>(13, -1)));

    // whereas signed versions obviously shouldn't.
    ASSERT_FALSE(test_compare<ir::Lt>(13, -1));
    ASSERT_FALSE(test_compare<ir::Lte>(13, -1));
    ASSERT_TRUE(test_compare<ir::Gt>(13, -1));
    ASSERT_TRUE(test_compare<ir::Gte>(13, -1));

    ASSERT_TRUE(test_compare<ir::Eq>(13, 13));
    ASSERT_FALSE(test_compare<ir::Neq>(13, 13));
    ASSERT_FALSE(test_compare<ir::Lt>(13, 13));
    ASSERT_TRUE(test_compare<ir::Lte>(13, 13));
    ASSERT_FALSE(test_compare<ir::Gt>(13, 13));
    ASSERT_TRUE(test_compare<ir::Gte>(13, 13));
}

TEST(X86Gen, Jump)
{
    ir::code code;

    auto fun = code(ir::Enter(code(ir::Fun(0, code(ir::Int(-64))))));
    auto rval = code(ir::RVal(fun));

    auto label = code(ir::Label());

    code(ir::Jump(label));
    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::RAX))), code(ir::Imm(69))));
    code(ir::Exit(fun));
    code(ir::Mark(label));
    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::RAX))), code(ir::Imm(42))));
    code(ir::Exit(fun));

    x86::function_gen<std::int64_t()> gen;
    code.pass(gen);
    ASSERT_EQ(42, gen.fun()());
}

template<class NODE, bool sgnd = true> bool test_irbranch(std::int64_t x, std::int64_t y)
{
    ir::code code;

    auto i64 = code(ir::Int(sgnd? -64 : 64)), b = code(ir::Int(0));
    auto fun = code(ir::Enter(code(ir::Fun(0, b, i64, i64))));
    auto rval = code(ir::RVal(fun));
    auto argx = code(ir::Arg(fun, 0));
    auto argy = code(ir::Arg(fun, 1));

    auto label = code(ir::Label());

    auto cond = code(NODE(code(ir::Reg(argx, ir::x86::id(X))), code(ir::Reg(argy, ir::x86::id(Y)))));
    code(ir::Branch(label, cond));
    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::AL))), code(ir::Imm(0))));
    code(ir::Exit(fun));
    code(ir::Mark(label));
    code(ir::Move(code(ir::Reg(rval, ir::x86::id(x86::AL))), code(ir::Imm(1))));
    code(ir::Exit(fun));

    x86::function_gen<bool(std::int64_t, std::int64_t)> gen;
    code.pass(gen);
    return gen.fun()(x, y);
}

TEST(X86Gen, Branch)
{
    // Exact same tests as in TEST(X86Gen, Compare)
    ASSERT_FALSE(test_irbranch<ir::Eq>(12345, 123));
    ASSERT_TRUE(test_irbranch<ir::Neq>(12345, 123));
    ASSERT_FALSE(test_irbranch<ir::Lt>(12345, 123));
    ASSERT_FALSE(test_irbranch<ir::Lte>(12345, 123));
    ASSERT_TRUE(test_irbranch<ir::Gt>(12345, 123));
    ASSERT_TRUE(test_irbranch<ir::Gte>(12345, 123));

    ASSERT_FALSE(test_irbranch<ir::Eq>(13, 666));
    ASSERT_TRUE(test_irbranch<ir::Neq>(13, 666));
    ASSERT_TRUE(test_irbranch<ir::Lt>(13, 666));
    ASSERT_TRUE(test_irbranch<ir::Lte>(13, 666));
    ASSERT_FALSE(test_irbranch<ir::Gt>(13, 666));
    ASSERT_FALSE(test_irbranch<ir::Gte>(13, 666));

    ASSERT_TRUE((test_irbranch<ir::Lt, false>(13, -1)));
    ASSERT_TRUE((test_irbranch<ir::Lte, false>(13, -1)));
    ASSERT_FALSE((test_irbranch<ir::Gt, false>(13, -1)));
    ASSERT_FALSE((test_irbranch<ir::Gte, false>(13, -1)));

    ASSERT_FALSE(test_irbranch<ir::Lt>(13, -1));
    ASSERT_FALSE(test_irbranch<ir::Lte>(13, -1));
    ASSERT_TRUE(test_irbranch<ir::Gt>(13, -1));
    ASSERT_TRUE(test_irbranch<ir::Gte>(13, -1));

    ASSERT_TRUE(test_irbranch<ir::Eq>(13, 13));
    ASSERT_FALSE(test_irbranch<ir::Neq>(13, 13));
    ASSERT_FALSE(test_irbranch<ir::Lt>(13, 13));
    ASSERT_TRUE(test_irbranch<ir::Lte>(13, 13));
    ASSERT_FALSE(test_irbranch<ir::Gt>(13, 13));
    ASSERT_TRUE(test_irbranch<ir::Gte>(13, 13));
}
