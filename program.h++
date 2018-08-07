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

#ifndef CODEGEN_PROGRAM_H
#define CODEGEN_PROGRAM_H

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
#include <unistd.h>
#endif

namespace codegen
{
    typedef std::uint8_t byte;

    // TODO: facilitate data as a part of this (badly named) entity, as there is otherwise no
    // portable way to guarantee that code and data are close enough to each other for shorter
    // relative immediate offsets
    class program
    {
        byte *_pages = nullptr;
        std::size_t _text_size, _size = 0;
        unsigned _refs = 1;

        static std::size_t page_size()
        {
            static auto n = sysconf(_SC_PAGESIZE);
            return n;
        }

        static std::size_t align(std::size_t p, std::size_t a = page_size())
        {
            return (p + page_size() - 1) & (~(std::size_t)0 - page_size() + 1);
        }

    public:

        struct exception
        {
        };

        program(const std::vector<byte> &text, const std::vector<byte> &data, std::size_t bss_size,
            const std::function<void(byte *, byte *, byte *)> &reloc = [](byte *, byte *, byte *) {})
        {
            if (text.empty() && data.empty() && !bss_size) return;
            _text_size = align(text.size());
            std::size_t bss_index = _text_size + align(data.size(), 64); // TODO: non-hard-coded cache line alignment
            _size = align(bss_index + bss_size);

#       ifdef CODEGEN_USE_MMAP

            void *block = mmap(0, _size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
            if (block == MAP_FAILED) throw exception();
            _pages = (byte *)block;

            std::copy(text.begin(), text.end(), _pages);
            std::copy(data.begin(), data.end(), _pages + _text_size);
            reloc(_pages, _pages + _text_size, _pages + bss_index);

            if (mprotect(_pages, _text_size, PROT_READ | PROT_EXEC) == -1) throw exception();

#       else

#            error "No implementation of codegen::program available."

#       endif

        }

        virtual ~program()
        {

#       ifdef CODEGEN_USE_MMAP

            if (_pages) munmap(_pages, _size);

#       endif

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
