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

#ifndef CODEGEN_REMAPPER_H
#define CODEGEN_REMAPPER_H

#include "ir.h++"

namespace codegen
{
    // This could be more efficient one day, good enough for now
    template<class OUT> class remapper
    {
        OUT &_out;
        std::map<ir::word, ir::word> _indices;
        ir::word _id = 0;

    public:

        remapper(OUT &out) : _out(out) { }

        template<class NODE> ir::word operator()(ir::word pos, const NODE &node)
        {
            NODE onode = node;
            for (unsigned i = 0; i < node.nargs(); ++i) if (node.is_id(i)) onode[i] = _indices[onode[i]];
            _indices[pos] = _out(onode);
            return pos;
        }

        template<class NODE> void operator()(const NODE &node)
        {
            NODE onode = node;
            for (unsigned i = 0; i < node.nargs(); ++i) if (node.is_id(i)) onode[i] = _indices[onode[i]];
            _out(onode);
        }

        template<class NODE> ir::word add(const NODE &node)
        {
            return (*this)(--_id, node);
        }

        void map(ir::word npos, ir::word opos)
        {
            _indices[opos] = _indices[npos];
        }
    };
}

#endif
