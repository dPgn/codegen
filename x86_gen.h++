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

        struct unsupported_ir
        {
        };

        class gen
        {
        protected:

            struct query_reg_mem
            {
                using rval = reg_mem;

                reg_mem operator()(const ir::code &code, ir::word pos, const ir::node &) const
                {
                    throw unsupported_node(code, pos);
                }

                reg_mem operator()(const ir::code &code, ir::word pos, const ir::Reg &node) const
                {
                    return ir::x86::integer_reg(node[1]);
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
                    auto dst = semantics(code, _dst);
                    auto src = semantics(code, node[1]);

                    if (src.is<ir::Imm>())
                        _a(INSTR(code.query_at(query_reg_mem(), _dst), src[0]));
                    else if (src.is<ir::Reg>())
                        _a(INSTR(code.query_at(query_reg_mem(), _dst), ir::x86::integer_reg(src[1])));
                    else if (dst.is<ir::Reg>())
                        _a(INSTR(ir::x86::integer_reg(dst[1]), code.query_at(query_reg_mem(), node[1])));
                    else
                        throw unsupported_ir();
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
                    auto dst = semantics(code, _dst);
                    if (!dst.is<ir::Reg>()) throw unsupported_ir();
                    _a(IMUL(ir::x86::integer_reg(dst[1]), code.query_at(query_reg_mem(), node[1])));
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

            struct gen_setcc
            {
                ir::word _dst;
                assembler &_a;
                bool _sgnd;

                gen_setcc(assembler &a, ir::word dst, bool sgnd) : _a(a), _dst(dst), _sgnd(sgnd) { }

                void operator()(const ir::code &code, ir::word pos, const ir::node &)
                {
                    throw unsupported_node(code, pos);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Eq &node)
                {
                    _a(SETE(code.query_at(query_reg_mem(), _dst)));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Neq &node)
                {
                    _a(SETNE(code.query_at(query_reg_mem(), _dst)));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Lt &node)
                {
                    if (_sgnd) _a(SETL(code.query_at(query_reg_mem(), _dst)));
                    else _a(SETB(code.query_at(query_reg_mem(), _dst)));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Lte &node)
                {
                    if (_sgnd) _a(SETLE(code.query_at(query_reg_mem(), _dst)));
                    else _a(SETBE(code.query_at(query_reg_mem(), _dst)));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Gt &node)
                {
                    if (_sgnd) _a(SETG(code.query_at(query_reg_mem(), _dst)));
                    else _a(SETA(code.query_at(query_reg_mem(), _dst)));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Gte &node)
                {
                    if (_sgnd) _a(SETGE(code.query_at(query_reg_mem(), _dst)));
                    else _a(SETAE(code.query_at(query_reg_mem(), _dst)));
                }
            };

            struct gen_jcc
            {
                label _l;
                assembler &_a;
                bool _sgnd;

                gen_jcc(assembler &a, label &l, bool sgnd) : _a(a), _l(l), _sgnd(sgnd) { }

                void operator()(const ir::code &code, ir::word pos, const ir::node &)
                {
                    throw unsupported_node(code, pos);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Eq &node)
                {
                    _a(JE(_l));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Neq &node)
                {
                    _a(JNE(_l));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Lt &node)
                {
                    if (_sgnd) _a(JL(_l));
                    else _a(JB(_l));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Lte &node)
                {
                    if (_sgnd) _a(JLE(_l));
                    else _a(JBE(_l));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Gt &node)
                {
                    if (_sgnd) _a(JG(_l));
                    else _a(JA(_l));
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Gte &node)
                {
                    if (_sgnd) _a(JGE(_l));
                    else _a(JAE(_l));
                }
            };

            struct gen_compare
            {
                assembler &_a;

                gen_compare(assembler &a) : _a(a) { }

                void operator()(const ir::code &code, ir::word pos, const ir::node &)
                {
                    throw unsupported_node(code, pos);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::compare &node)
                {
                    // TODO:
                    // This is copypasted (with small adjustments) from gen_integer_arithmetic.
                    // Should probably be refactored into one common (template) function.

                    auto d = semantics(code, pos)[0];
                    auto s = semantics(code, pos)[1];
                    auto dst = semantics(code, d);
                    auto src = semantics(code, s);

                    if (src.is<ir::Imm>())
                        _a(CMP(code.query_at(query_reg_mem(), d), src[0]));
                    else if (src.is<ir::Reg>())
                        _a(CMP(code.query_at(query_reg_mem(), d), ir::x86::integer_reg(src[1])));
                    else if (dst.is<ir::Reg>())
                        _a(CMP(ir::x86::integer_reg(dst[1]), code.query_at(query_reg_mem(), s)));
                    else
                        throw unsupported_ir();
                }
            };

            assembler _a;
            std::map<ir::word, label> _labels;

        public:

            void operator()(const ir::code &code, ir::word pos, const ir::node &) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Move &node)
            {
                auto dst = semantics(code, node[0]);
                auto src = semantics(code, node[1]);
                auto dst_type = dst.type();
                if (dst_type.is<ir::Int>())
                {
                    if (src.is<ir::arithmetic>())
                    {
                        if (node[0] != src[0]) (*this)(code, pos, ir::Move(node[0], src[0]));
                        code.pass_at(gen_integer_arithmetic(_a, node[0]), node[1]);
                    }
                    else if (src.is<ir::compare>())
                    {
                        code.pass_at(gen_compare(_a), node[1]);
                        code.pass_at(gen_setcc(_a, node[0], semantics(code, src[0]).is_signed()), node[1]);
                    }
                    else if (dst.is<ir::Reg>())
                    {
                        auto dreg = ir::x86::integer_reg(dst[1]);
                        if (src.is<ir::Reg>())
                            _a(MOV(dreg, ir::x86::integer_reg(src[1])));
                        else if (src.is<ir::Imm>())
                            _a(MOV(dreg, src[0]));
                    }
                }
                // TODO: more than I'd like to admit
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Label &)
            {
                _labels[pos] = label(_a);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Mark &node)
            {
                _a(_labels[node[0]]);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Jump &node)
            {
                _a(JMP(_labels[node[0]]));
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Branch &node)
            {
                auto cond = semantics(code, node[1]);
                if (cond.is<ir::compare>())
                {
                    code.pass_at(gen_compare(_a), node[1]);
                    code.pass_at(gen_jcc(_a, _labels[node[0]], semantics(code, cond[0]).is_signed()), node[1]);
                }
                else
                {
                    _a(CMP(code.query_at(query_reg_mem(), node[1]), 0));
                    _a(JNZ(_labels[node[0]]));
                }
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Exit &node)
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
