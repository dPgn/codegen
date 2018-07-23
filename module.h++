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

#ifndef CODEGEN_MODULE_H
#define CODEGEN_MODULE_H

#include "function.h++"

namespace codegen
{
	class module
	{
	protected:

		std::vector<byte> code;
	};

	template<class T> class function_module { };

	template<class R, class... A> class function_module<R(A...)> : public module
	{

	public:

		function_module() { }

		function_module(const std::vector<byte> &content)
		{
			code = content;
		}

		function<R(A...)> link_function()
		{
			return function<R(A...)>(new codeseg(code));
		}
	};
}

#endif
