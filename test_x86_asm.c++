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
