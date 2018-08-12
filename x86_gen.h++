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

#ifndef CODEGEN_X86_GEN_H
#define CODEGEN_X86_GEN_H

#include "x86_ir.h++"
#include "semantics.h++"

namespace codegen
{
    namespace x86
    {
        struct unsupported_node
        {
            const ir::word _id;
            const std::string _name;
        };

        class gen
        {
        protected:

            assembler _a;

        public:

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                throw unsupported_node { node.id(), node.name() };
            }

            void operator()(const ir::code &code, ir::word pos, ir::Imm node) { }

            void operator()(const ir::code &code, ir::word pos, ir::Move node)
            {

                if (ir::x86::is_integer_reg(node[0]))
                    if (ir::x86::is_integer_reg(node[1]))
                        _a(MOV(ir::x86::integer_reg(node[0]), ir::x86::integer_reg(node[1])));
                    else if (semantics(code, node[1]).is_immediate())
                        _a(MOV(ir::x86::integer_reg(node[0]), semantics(code, node[1]).int64()));
            }

            void operator()(const ir::code &code, ir::word pos, ir::Ret node)
            {
                _a(RET());
            }
        };

        template<class T> class function_gen { };

        template<class R, class... ARGS> class function_gen<R(ARGS...)> : public gen
        {
        public:

            function<R(ARGS...)> fun()
            {
                return _a.assemble_function<R(ARGS...)>().link();
            }
        };
    }
}

#endif
