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

#ifndef CODEGEN_CODESEG_H
#define CODEGEN_CODESEG_H

// Not tested, yet!
// TODO: Test on Linux
#ifdef __linux__
#define CODEGEN_USE_MMAP
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define CODEGEN_USE_MMAP
#endif

// TODO: Windows support

#include <vector>
#include <functional>

#ifdef CODEGEN_USE_MMAP
#include <sys/mman.h>
#endif

namespace codegen
{
	typedef std::uint8_t byte;

	class codeseg
	{
		byte *_pages = nullptr;
		std::size_t _size = 0;
		unsigned _refs = 1;

	public:

		struct exception
		{
		};

		codeseg(const std::vector<byte> &code, const std::function<void(byte *)> &reloc = [](byte *) {})
		{
			if (code.empty()) return;
			_size = code.size();

#		ifdef CODEGEN_USE_MMAP

			void *block = mmap(0, _size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
			if (block == MAP_FAILED) throw exception();
			_pages = (byte *)block;

			std::copy(code.begin(), code.end(), _pages);
			reloc(_pages);

			if (mprotect(_pages, _size, PROT_READ | PROT_EXEC) == -1) throw exception();

#		else

#			error "No implementation of codegen::codeseg available."

#		endif

		}

		virtual ~codeseg()
		{

#		ifdef CODEGEN_USE_MMAP

			if (_pages) munmap(_pages, _size);

#		endif

		}

		operator byte *() const
		{
			return _pages;
		}

		void add_ref()
		{
			++_refs;
		}

		void remove_ref()
		{
			if (--_refs) delete this;
		}
	};
}

#endif
