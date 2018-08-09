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
    public:

        struct reloc
        {
            unsigned _refs = 1;

            virtual ~reloc() { }

            virtual void resolve(std::map<std::intptr_t, byte *> &mapping, byte *text, byte *data, byte *bss) = 0;

            virtual void write(byte *text, byte *data, std::map<std::intptr_t, byte *> &mapping) = 0;
        };

    protected:

        std::vector<byte> _text;
        std::vector<byte> _data;
        std::size_t _bss_size;
        reloc *_reloc = nullptr;

    public:

        module() { }

        module(const module &other)
        {
            _text = other._text;
            _data = other._data;
            _bss_size = other._bss_size;
            if (_reloc = other._reloc) ++_reloc->_refs;
        }

        module(module &&other)
        {
            _text = other._text;
            _data = other._data;
            _bss_size = other._bss_size;
            _reloc = other._reloc;
            other._reloc = nullptr;
        }

        module &operator=(const module &other)
        {
            _text = other._text;
            _data = other._data;
            _bss_size = other._bss_size;
            if (_reloc = other._reloc) ++_reloc->_refs;
            return *this;
        }

        module &operator=(module &&other)
        {
            _text = other._text;
            _data = other._data;
            _bss_size = other._bss_size;
            _reloc = other._reloc;
            other._reloc = nullptr;
            return *this;
        }

        ~module()
        {
            if (_reloc && !--_reloc->_refs) delete _reloc;
        }
    };

    template<class T> struct linkable_module : module
    {
        linkable_module() { }

        linkable_module(const std::vector<byte> &text, const std::vector<byte> &data, std::size_t bss_size = 0, module::reloc *reloc = nullptr)
        {
            _text = text;
            _data = data;
            _bss_size = bss_size;
            if (_reloc = reloc) ++reloc->_refs;
        }

        T link()
        {
            if (!_reloc) return T(new program(_text, _data, _bss_size));
            auto reloc = [&](byte *text, byte *data, byte *bss)
            {
                std::map<std::intptr_t, byte *> mapping;
                _reloc->resolve(mapping, text, data, bss);
                _reloc->write(text, data, mapping);
            };
            return T(new program(_text, _data, _bss_size, reloc));
        }
    };

    template<class T> class function_module { };

    template<class R, class... A> struct function_module<R(A...)> : linkable_module<function<R(A...)>>
    {
        function_module() { }

        function_module(const std::vector<byte> &text, const std::vector<byte> &data, std::size_t bss_size = 0, module::reloc *reloc = nullptr)
            : linkable_module<function<R(A...)>>(text, data, bss_size, reloc) { }
    };
}

#endif
