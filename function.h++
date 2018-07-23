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

#ifndef CODEGEN_FUNCTION_H
#define CODEGEN_FUNCTION_H

#include "codeseg.h++"

namespace codegen
{
	template<class T> class function { };

	// TODO: Rule of 5
	template<class R, class... ARGS> class function<R(ARGS...)>
	{
		codeseg *code = nullptr;
		std::size_t offset = 0;

	public:

		function(const function &other)
		{
			*this = other;
		}

		function(function &&other)
		{
			*this = other;
		}

		function(codeseg *code = nullptr, std::size_t offset = 0)
		{
			this->code = code;
			this->offset = offset;
		}

		virtual ~function()
		{
			if (code) code->remove_ref();
		}

		R operator()(ARGS... args)
		{
			return reinterpret_cast<R(*)(ARGS...)>((byte *)*code + offset)(args...);
		}

		function &operator=(const function &other)
		{
			code = other.code;
			offset = other.offset;
			code->add_ref();
			return *this;
		}

		function &operator=(function &&other)
		{
			code = other.code;
			offset = other.offset;
			other.code = nullptr;
			return *this;
		}
	};
}

#endif
