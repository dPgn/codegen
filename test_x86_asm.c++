#include "x86_asm.h++"

bool test_x86_asm()
{
	std::cout << "x86::assembler test" << std::endl;

	std::cout << "Create a dummy return-only function using x86::assembler:" << std::flush;

	codegen::x86::assembler<64> a;
	codegen::function<void()> fun;

	try
	{
		a.append_ret();
		fun = a.assemble<void()>();
	}
	catch (codegen::x86::exception)
	{
		std::cout << "failed" << std::endl;
		return false;
	}

	std::cout << "OK" << std::endl;

	std::cout << "And run it:" << std::flush;

	fun();

	std::cout << "OK" << std::endl;

	return true;
}
