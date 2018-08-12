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

#ifndef CODEGEN_SEMANTICS_H
#define CODEGEN_SEMANTICS_H

#include "ir.h++"

namespace codegen
{
    class semantics
    {
        template<class WHAT> struct is_query
        {
            using rval = bool;

            rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
            {
                return false;
            }

            rval operator()(const ir::code &code, ir::word pos, const WHAT &node) const
            {
                return true;
            }
        };

        struct int64_query
        {
            using rval = std::int_least64_t;

            template<class NODE> rval operator()(const ir::code &code, ir::word pos, const NODE &node) const
            {
                // TODO: something nice
                throw 0;
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Imm &node) const
            {
                return node[0];
            }
        };

        struct type_query
        {
        };

        const ir::code &_code;
        ir::word _pos;

    public:

        semantics(const ir::code &code, ir::word pos) : _code(code), _pos(pos) { }

        template<class WHAT> bool is() const
        {
            auto pos = _pos;
            return _code.query(is_query<WHAT>(), pos);
        }

        std::int_least64_t int64() const
        {
            auto pos = _pos;
            return _code.query(int64_query(), pos);
        }
    };

}

#endif
