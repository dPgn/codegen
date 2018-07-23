#include <iostream>

#include "codeseg.h++"
#include "x86_asm.h++"

static bool test_codeseg()
{
	// codeseg is a wrapper for memory pages filled with dynamically generated code and marked executable.
	// Its purpose is to hide the necessary operating system APIs.  Architectures without NX bit could all
	// run one portable implementation of codeseg. However, all relevant desktop and mobile processors
	// already have this feature, and it's unlikely to go anywhere.

	std::cout << "codeseg test" << std::endl;
	std::cout << "Create a dummy parameterless return-only function and call it: " << std::flush;

	// TODO: #ifdef X86/X64 or something like that
	std::vector<codegen::byte> ret_code { 0xc3 };

	try
	{
		codegen::codeseg seg(ret_code);
		((void(*)())(void *)seg)();
	}
	catch (codegen::codeseg::exception)
	{
		std::cout << "failed" << std::endl;
		return false;
	}

	std::cout << "OK" << std::endl;

	// The relocation logic is passed to codeseg as a lambda to be executed before the code pages have
	// been marked read and execute only. Although the mechanism is intended for relocations, the lambda
	// can freely modify the code. Therefore, we can test the functionality by passing malformed code
	// (plain zeros) and letting the relocation lambda overwrite it with our return-only code from above.

	std::cout << "Test the relocation interface: " << std::flush;

	std::vector<codegen::byte> null_code(ret_code.size(), 0);

	try
	{
		// Dummy "relocation" function that overwrites the null code with the return-only code:
		auto reloc = [&ret_code](codegen::byte *code) { std::copy(ret_code.begin(), ret_code.end(), code); };

		codegen::codeseg seg(null_code, reloc);
		((void(*)())(void *)seg)();
	}
	catch (codegen::codeseg::exception)
	{
		std::cout << "failed" << std::endl;
		return false;
	}

	std::cout << "OK" << std::endl << std::endl;

	std::cout << "Test function object generation: " << std::flush;

	codegen::function<void(void)> fun;

	try
	{
		codegen::codeseg seg(ret_code);
		fun = seg.get_function<void(void)>();
	}
	catch (codegen::codeseg::exception)
	{
		std::cout << "failed" << std::endl;
		return false;
	}

	fun();

	return true;
}

int main(int argc, const char *argv[])
{
	std::cout << "codegen test" << std::endl << std::endl;

	if (!test_codeseg()) return 1;

	return 0;
}
