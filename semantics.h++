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

        struct funarg_query
        {
            using rval = ir::word;

            int _index;

            funarg_query(int index) : _index(index) { } // -1 = return value

            rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
            {
                return -1;
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Fun &node) const
            {
                return node[1 + _index];
            }
        };

        struct type_query
        {
            using rval = ir::word;

            rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
            {
                return -1;
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Reg &node) const
            {
                return semantics(code, node[0]).type();
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::RVal &node) const
            {
                auto funtype = semantics(code, node[0]).type();
                return code.query(funarg_query(-1), funtype);
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Arg &node) const
            {
                auto funtype = semantics(code, node[0]).type();
                return code.query(funarg_query(node[1]), funtype);
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Enter &node) const
            {
                return node[0];
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

        const ir::code &_code;
        ir::word _pos;

    public:

        semantics(const ir::code &code, ir::word pos) : _code(code), _pos(pos) { }

        ir::word operator[](ir::word k) const
        {
            return _code.arg(_pos, k);
        }

        template<class WHAT> bool is() const
        {
            auto pos = _pos;
            return _code.query(is_query<WHAT>(), pos);
        }

        ir::word type() const
        {
            auto pos = _pos;
            return _code.query(type_query(), pos);
        }

        std::int_least64_t int64() const
        {
            auto pos = _pos;
            return _code.query(int64_query(), pos);
        }
    };

}

#endif
