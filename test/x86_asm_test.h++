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

namespace x86 = codegen::x86;

TEST(X86Asm, PlainReturn)
{
	// Generate the assembler object
	// The subarchitecture is, by default, the host architecture. This entire test should be
	// disabled for non-x86 architectures when their support is added.
	x86::assembler a;

	// Insert RET instruction
	ASSERT_NO_THROW(a(x86::RET()));

	// Assemble
	// A function_module<R(A...)> object encapsulates a non-linked function with a specific
	// signature.
	codegen::function_module<void()> code;
	ASSERT_NO_THROW(code = a.assemble_function<void()>());

	// "Link" a runnable function
	// Linking without an argument (external symbol table) only works with functions like this that
	// do not reference any external symbols. It effectively copies the assembled code as it is
	// into a valid codeseg and wrapped into a valid callable function<R(A...)> object.
	codegen::function<void()> fun;
	ASSERT_NO_THROW(fun = code.link());

	// Run the runnable function
	// We make a death test here, because, if the code above fails, calling fun() will most likely
	// segfault. As gtest can only assert that a piece of code dies, but not that it doesn't (which
	// certainly seems odd), we exit after the call to fun() and expect a normal exit.
	ASSERT_EXIT({ fun(); exit(0); }, testing::ExitedWithCode(0), "");
}

// The first two integer argument registers
// Everything except Windows (that I'm aware of) uses System V ABI.
// Windows is actually not supported by the library, yet, but the calling convention is already
// included here to avoid confusion when the support is eventually implemented.
#ifdef _WIN32
constexpr auto X = x86::RCX, Y = x86::RDX;
#else
constexpr auto X = x86::RDI, Y = x86::RSI;
#endif

template<class INSTR> std::int64_t test_basic_binary_instruction(std::int64_t x, std::int64_t y)
{
	x86::assembler a;

	// To make ADC and SBB different from ADD and SUB
	a(x86::STC());
	a(INSTR(X, Y));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	// Assemble, link, and execute
	return a.assemble_function<std::int64_t(std::int64_t, std::int64_t)>().link()(x, y);
}

TEST(X86Asm, BasicBinaryInstructions)
{
	EXPECT_EQ(55, test_basic_binary_instruction<x86::ADD>(13, 42));
	EXPECT_EQ(47, test_basic_binary_instruction<x86::OR>(13, 42));
	EXPECT_EQ(56, test_basic_binary_instruction<x86::ADC>(13, 42));
	EXPECT_EQ(-30, test_basic_binary_instruction<x86::SBB>(13, 42));
	EXPECT_EQ(8, test_basic_binary_instruction<x86::AND>(13, 42));
	EXPECT_EQ(-29, test_basic_binary_instruction<x86::SUB>(13, 42));
	EXPECT_EQ(39, test_basic_binary_instruction<x86::XOR>(13, 42));
}

TEST(X86Asm, AddressModes)
{
	std::int64_t n = 25;

	x86::assembler a;

	a(x86::ADD(X, x86::DS[Y]));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t *)>().link()(17, &n)) << "Source = [Y]";

	a.clear();

	// xBP is a special case
	a(x86::MOV(x86::RAX, x86::RBP));
	a(x86::MOV(x86::RBP, Y));
	a(x86::ADD(X, x86::DS[x86::RBP]));
	a(x86::MOV(x86::RBP, x86::RAX));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t *)>().link()(17, &n)) << "Source = [RBP]";

	a.clear();

	// as is xSP (certainly)
	a(x86::MOV(x86::RAX, x86::RSP));
	a(x86::MOV(x86::RSP, Y));
	a(x86::ADD(X, x86::DS[x86::RSP]));
	a(x86::MOV(x86::RSP, x86::RAX));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t *)>().link()(17, &n)) << "Source = [RBP]";

	a.clear();

	// and R13
	a(x86::MOV(x86::RAX, x86::R13));
	a(x86::MOV(x86::R13, Y));
	a(x86::ADD(X, x86::DS[x86::R13]));
	a(x86::MOV(x86::R13, x86::RAX));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t *)>().link()(17, &n)) << "Source = [RBP]";

	a.clear();

	a(x86::ADD(x86::DS[X], Y));
	a(x86::RET());

	a.assemble_function<void(std::int64_t *, std::int64_t)>().link()(&n, 17);

	ASSERT_EQ(42, n) << "Destination = [X]";

	a.clear();

	a(x86::SUB(X, 13));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t)>().link()(55)) << "Source = Immediate";

	a.clear();

	a(x86::ADD(x86::DS.QWORD[X], 13));
	a(x86::RET());

	a.assemble_function<void(std::int64_t *)>().link()(&n);

	ASSERT_EQ(55, n) << "Destination = [X], Source = Immediate";
}

template<class INSTR> bool test_branch(std::int64_t x, std::int64_t y)
{
	x86::assembler a;
	x86::label yes(a);

	a(x86::CMP(X, Y));
	a(INSTR(yes));
	a(x86::XOR(x86::RAX, x86::RAX));
	a(x86::RET());
	a(yes);
	a(x86::MOV(x86::RAX, 1));
	a(x86::RET());

	return a.assemble_function<bool(std::int64_t, std::int64_t)>().link()(x, y);
}

TEST(X86Asm, Branches)
{
	ASSERT_TRUE(test_branch<x86::JE>(42, 42));
	ASSERT_TRUE(test_branch<x86::JNE>(42, 13));
	ASSERT_TRUE(test_branch<x86::JC>(42, -1));
	ASSERT_TRUE(test_branch<x86::JNC>(42, 1));
	ASSERT_TRUE(test_branch<x86::JB>(42, -1));
	ASSERT_TRUE(test_branch<x86::JNB>(42, 1));
	ASSERT_TRUE(test_branch<x86::JG>(42, 13));
	ASSERT_FALSE(test_branch<x86::JG>(42, 42));
	ASSERT_FALSE(test_branch<x86::JG>(13, 42));
	ASSERT_TRUE(test_branch<x86::JGE>(42, 13));
	ASSERT_TRUE(test_branch<x86::JGE>(42, 42));
	ASSERT_FALSE(test_branch<x86::JGE>(13, 42));
	ASSERT_TRUE(test_branch<x86::JL>(13, 42));
	ASSERT_FALSE(test_branch<x86::JL>(42, 13));
	ASSERT_TRUE(test_branch<x86::JLE>(13, 42));
	ASSERT_TRUE(test_branch<x86::JLE>(42, 42));
	ASSERT_FALSE(test_branch<x86::JLE>(42, 13));
}

TEST(X86Asm, JmpCall)
{
	x86::assembler a;
	x86::label target(a);

	a(x86::JMP(target));
	a(x86::XOR(x86::RAX, x86::RAX));
	a(x86::RET());
	a(target);
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t)>().link()(42));

	a.clear();

	a(x86::CALL(target));
	a(x86::XOR(x86::RAX, x86::RAX));
	a(x86::RET());
	a(target);
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(0, a.assemble_function<std::int64_t(std::int64_t)>().link()(42));
}

TEST(X86Asm, MulDiv)
{
	x86::assembler a;

	a(x86::MOV(x86::RAX, X));
	a(x86::MUL(Y));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t)>().link()(6, 7)) << "MUL Y";

	a.clear();

	a(x86::MOV(x86::RAX, X));
	a(x86::IMUL(Y));
	a(x86::RET());

	ASSERT_EQ(69, a.assemble_function<std::int64_t(std::int64_t, std::int64_t)>().link()(23, 3)) << "IMUL Y";

	a.clear();

	a(x86::IMUL(X, Y));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(44, a.assemble_function<std::int64_t(std::int64_t, std::int64_t)>().link()(4, 11)) << "IMUL X, Y";

	a.clear();

	a(x86::IMUL(x86::RAX, X, 42));
	a(x86::RET());

	ASSERT_EQ(420, a.assemble_function<std::int64_t(std::int64_t)>().link()(10)) << "IMUL EAX, X, 42";
}

template<class INSTR> std::uint64_t test_shift_imm3(std::uint64_t x)
{
	x86::assembler a;

	a(x86::CLC());
	a(INSTR(X, 3));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	return a.assemble_function<std::uint64_t(std::uint64_t)>().link()(x);
}

TEST(X86Asm, Shifts)
{
	ASSERT_EQ(416, test_shift_imm3<x86::SHL>(52));
	ASSERT_EQ(52, test_shift_imm3<x86::SHR>(416));
	ASSERT_EQ(408, test_shift_imm3<x86::SAL>(51));
	ASSERT_EQ(51, test_shift_imm3<x86::SAR>(408));
	ASSERT_EQ(328, test_shift_imm3<x86::ROL>(41));
	ASSERT_EQ(41, test_shift_imm3<x86::ROR>(328));
	ASSERT_EQ(184, test_shift_imm3<x86::RCL>(23));
	ASSERT_EQ(23, test_shift_imm3<x86::RCR>(184));

	ASSERT_EQ(0, test_shift_imm3<x86::SHL>(0x2000000000000000ULL));
	ASSERT_EQ(0, test_shift_imm3<x86::SHR>(1));
	ASSERT_EQ(1, test_shift_imm3<x86::ROL>(0x2000000000000000ULL));
	ASSERT_EQ(0x2000000000000000ULL, test_shift_imm3<x86::ROR>(1));

	ASSERT_EQ(0x1fffffffffffffffULL, test_shift_imm3<x86::SHR>(0xffffffffffffffffULL));
	ASSERT_EQ(0xffffffffffffffffULL, test_shift_imm3<x86::SAR>(0xffffffffffffffffULL));
	ASSERT_EQ(0xfffffffffffffff8ULL, test_shift_imm3<x86::SHL>(0xffffffffffffffffULL));
	ASSERT_EQ(0xffffffffffffffffULL, test_shift_imm3<x86::ROL>(0xffffffffffffffffULL));
	ASSERT_EQ(0xfffffffffffffffbULL, test_shift_imm3<x86::RCL>(0xffffffffffffffffULL));
	ASSERT_EQ(0xffffffffffffffffULL, test_shift_imm3<x86::ROR>(0xffffffffffffffffULL));
	ASSERT_EQ(0xdfffffffffffffffULL, test_shift_imm3<x86::RCR>(0xffffffffffffffffULL));

	x86::assembler a;

#	ifndef _WIN32
	a(x86::MOV(x86::RCX, X));
#	endif

	a(x86::SHL(Y, x86::CL));
	a(x86::MOV(x86::RAX, Y));
	a(x86::RET());

	ASSERT_EQ(420, a.assemble_function<std::uint64_t(std::uint64_t, std::uint64_t)>().link()(2, 105));

	a.clear();

	a(x86::SHR(X, 1));
	a(x86::MOV(x86::RAX, X));
	a(x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::uint64_t(std::uint64_t)>().link()(84));
}

TEST(X86Asm, Data)
{
	x86::assembler a;
	x86::global var;

	a(x86::MOV(x86::RAX, x86::DS[var]));
	a(x86::RET());
	a.data();
	a(var);
	a(x86::DQ(123456));

	codegen::function_module<std::int64_t()> module;
	try
	{
 		module = a.assemble_function<std::int64_t()>();
	}
	catch (x86::argument_mismatch ex)
	{
		std::cout << "Argument mismatch: " << ex.msg() << std::endl;
	}

	ASSERT_EQ(123456, module.link()());
}
