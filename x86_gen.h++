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
            const ir::vnode *_node;

            unsupported_node(const ir::code &code, ir::word pos) : _node(code.read(pos)) { }

            ~unsupported_node()
            {
                delete _node;
            }
        };

        class gen
        {
        protected:

            assembler _a;

            struct query_reg_mem
            {
                using rval = reg_mem;

                reg_mem operator()(const ir::code &code, ir::word pos, const ir::node &) const
                {
                    throw unsupported_node(code, pos);
                }

                reg_mem operator()(const ir::code &code, ir::word pos, const ir::Ld &node) const
                {
                    if (ir::x86::is_integer_reg(node[0]))
                        return DS[ir::x86::integer_reg(node[0])];
                }
            };

            // An important part of this is to convert the IR node into a two-address instruction.
            // The first operand of the arithmetic operation node is ignored, and _dst is used,
            // instead.
            struct gen_integer_arithmetic
            {
                ir::word _dst;
                assembler &_a;

                gen_integer_arithmetic(assembler &a, ir::word dst) : _a(a), _dst(dst) { }

                void operator()(const ir::code &code, ir::word pos, const ir::node &)
                {
                    throw unsupported_node(code, pos);
                }

                template<class INSTR, class NODE> void encode_binary(const ir::code &code, const NODE &node)
                {
                    if (ir::x86::is_integer_reg(_dst))
                        if (semantics(code, node[1]).is<ir::Imm>())
                            _a(INSTR(ir::x86::integer_reg(_dst), semantics(code, node[1]).int64()));
                        else if (ir::x86::is_integer_reg(node[1]))
                            _a(INSTR(ir::x86::integer_reg(_dst), ir::x86::integer_reg(node[1])));
                        else
                        {
                            auto t = node[1];
                            _a(INSTR(ir::x86::integer_reg(_dst), code.query(query_reg_mem(), t)));
                        }
                    // TODO: a lot
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Add &node)
                {
                    encode_binary<ADD, ir::Add>(code, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Sub &node)
                {
                    encode_binary<SUB, ir::Sub>(code, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Mul &node)
                {
                    encode_binary<IMUL, ir::Mul>(code, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Xor &node)
                {
                    encode_binary<XOR, ir::Xor>(code, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::And &node)
                {
                    encode_binary<AND, ir::And>(code, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Or &node)
                {
                    encode_binary<OR, ir::Or>(code, node);
                }

                // Divisions must be converted to long versions before passing them to x86::gen,
                // because higher level stages should be aware of such conversion.
                void operator()(const ir::code &code, ir::word pos, const ir::Div &node)
                {
                    throw unsupported_node(code, pos);
                }
            };

        public:

            void operator()(const ir::code &code, ir::word pos, const ir::node &)
            {
                throw unsupported_node(code, pos);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Imm &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::arithmetic &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Move &node)
            {
                if (ir::x86::is_integer_reg(node[0]))
                    if (ir::x86::is_integer_reg(node[1]))
                        _a(MOV(ir::x86::integer_reg(node[0]), ir::x86::integer_reg(node[1])));
                    else if (semantics(code, node[1]).is<ir::Imm>())
                        _a(MOV(ir::x86::integer_reg(node[0]), semantics(code, node[1]).int64()));
                    else if (semantics(code, node[1]).is<ir::arithmetic>())
                    {
                        ir::word t = node[1];
                        ir::vnode &val = *code.read(t);
                        if (val[0] != node[0]) (*this)(code, pos, ir::Move(node[0], val[0]));
                        auto g = gen_integer_arithmetic(_a, node[0]);
                        code.pass(g, t = node[1]);
                    }
                // TODO: more than I'd like to admit
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Ret &node)
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
