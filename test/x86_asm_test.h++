TEST(X86Asm, PlainReturn)
{
	// Generate the assembler object
	// The subarchitecture is, by default, the host architecture. This entire test should be
	// disabled for non-x86 architectures when their support is added.
	codegen::x86::assembler a;

	// Insert RET instruction
	ASSERT_NO_THROW(a(codegen::x86::RET()));

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
	ASSERT_NO_THROW(fun = code.link_function());

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
constexpr auto X = codegen::x86::RCX, Y = codegen::x86::RDX;
#else
constexpr auto X = codegen::x86::RDI, Y = codegen::x86::RSI;
#endif

template<class INSTR> std::int64_t test_basic_binary_instruction(std::int64_t x, std::int64_t y)
{
	codegen::x86::assembler a;

	// To make ADC and SBB different from ADD and SUB
	a(codegen::x86::STC());

	a(INSTR(X, Y));
	a(codegen::x86::MOV(codegen::x86::RAX, X));
	a(codegen::x86::RET());

	// Assemble, link, and execute
	return a.assemble_function<std::int64_t(std::int64_t, std::int64_t)>().link_function()(x, y);
}

TEST(X86Asm, BasicBinaryInstructions)
{
	EXPECT_EQ(55, test_basic_binary_instruction<codegen::x86::ADD>(13, 42));
	EXPECT_EQ(47, test_basic_binary_instruction<codegen::x86::OR>(13, 42));
	EXPECT_EQ(56, test_basic_binary_instruction<codegen::x86::ADC>(13, 42));
	EXPECT_EQ(-30, test_basic_binary_instruction<codegen::x86::SBB>(13, 42));
	EXPECT_EQ(8, test_basic_binary_instruction<codegen::x86::AND>(13, 42));
	EXPECT_EQ(-29, test_basic_binary_instruction<codegen::x86::SUB>(13, 42));
	EXPECT_EQ(39, test_basic_binary_instruction<codegen::x86::XOR>(13, 42));
}

TEST(X86Asm, AddressModes)
{
	std::int64_t n = 25;

	codegen::x86::assembler a;

	a(codegen::x86::ADD(X, codegen::x86::DS[Y]));
	a(codegen::x86::MOV(codegen::x86::RAX, X));
	a(codegen::x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t *)>().link_function()(17, &n)) << "Source = [Y]";

	a.clear();

	// xBP is a special case
	a(codegen::x86::MOV(codegen::x86::RAX, codegen::x86::RBP));
	a(codegen::x86::MOV(codegen::x86::RBP, Y));
	a(codegen::x86::ADD(X, codegen::x86::DS[codegen::x86::RBP]));
	a(codegen::x86::MOV(codegen::x86::RBP, codegen::x86::RAX));
	a(codegen::x86::MOV(codegen::x86::RAX, X));
	a(codegen::x86::RET());

	ASSERT_EQ(42, a.assemble_function<std::int64_t(std::int64_t, std::int64_t *)>().link_function()(17, &n)) << "Source = [RBP]";

	a.clear();

	a(codegen::x86::ADD(codegen::x86::DS[X], Y));
	a(codegen::x86::RET());

	a.assemble_function<void(std::int64_t *, std::int64_t)>().link_function()(&n, 17);

	ASSERT_EQ(42, n) << "Destination = [X]";
}
