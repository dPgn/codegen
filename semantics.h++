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

        struct type_query
        {
            using rval = ir::word;

            rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
            {
                return -1;
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::arithmetic &node) const
            {
                return semantics(code, semantics(code, pos)[0]).type().pos();
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Reg &node) const
            {
                return semantics(code, node[0]).type().pos();
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Temp &node) const
            {
                return node[0];
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::RVal &node) const
            {
                return semantics(code, node[0]).type()[1];
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Arg &node) const
            {
                return semantics(code, node[0]).type()[2 + node[1]];
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Enter &node) const
            {
                return node[0];
            }
        };

        struct is_signed_query
        {
            using rval = bool;

            rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
            {
                return semantics(code, pos).type().is_signed();
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Int &node) const
            {
                return node[0] < 0;
            }

            rval operator()(const ir::code &code, ir::word pos, const ir::Float &node) const
            {
                return true;
            }
        };

        const ir::code &_code;
        ir::word _pos;

    public:

        semantics(const ir::code &code, ir::word pos) : _code(code), _pos(pos)
        {
            if (pos < 0) throw 0; // TODO: an actual exception
        }

        ir::word pos() const
        {
            return _pos;
        }

        ir::word operator[](ir::word k) const
        {
            return _code.arg(_pos, k);
        }

        template<class WHAT> bool is() const
        {
            return _code.query_at(is_query<WHAT>(), _pos);
        }

        semantics type() const
        {
            return semantics(_code, _code.query_at(type_query(), _pos));
        }

        bool is_signed() const
        {
            return _code.query_at(is_signed_query(), _pos);
        }
    };

}

#endif
