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

#include "program.h++"

namespace codegen
{
    template<class T> class function { };

    // TODO: Rule of 5
    template<class R, class... ARGS> class function<R(ARGS...)>
    {
        program *_code = nullptr;
        std::size_t _offset = 0;

    public:

        function(const function &other)
        {
            *this = other;
        }

        function(function &&other)
        {
            *this = other;
        }

        function(program *code = nullptr, std::size_t offset = 0)
        {
            _code = code;
            _offset = offset;
        }

        virtual ~function()
        {
            if (_code) _code->remove_ref();
        }

        R operator()(ARGS... args)
        {
            return reinterpret_cast<R(*)(ARGS...)>((byte *)*_code + _offset)(args...);
        }

        function &operator=(const function &other)
        {
            _code = other._code;
            _offset = other._offset;
            _code->add_ref();
            return *this;
        }

        function &operator=(function &&other)
        {
            _code = other._code;
            _offset = other._offset;
            other._code = nullptr;
            return *this;
        }
    };
}

#endif
