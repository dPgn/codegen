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

#ifndef CODEGEN_X86_RTL
#define CODEGEN_X86_RTL

#include "semantics.h++"
#include "remapper.h++"

namespace codegen
{
    namespace x86
    {
        struct rtl_analysis
        {
            std::set<ir::word> _regs;

            struct handle_src
            {
                rtl_analysis &_a;

                void operator()(const ir::code &code, ir::word pos, const ir::Reg &node) { }
                void operator()(const ir::code &code, ir::word pos, const ir::Arg &node) { }
                void operator()(const ir::code &code, ir::word pos, const ir::Imm &node) { }
                void operator()(const ir::code &code, ir::word pos, const ir::Temp &node) { }
                void operator()(const ir::code &code, ir::word pos, const ir::typecon &node) { }
                void operator()(const ir::code &code, ir::word pos, const ir::Enter &node) { }
                void operator()(const ir::code &code, ir::word pos, const ir::Exit &node) { }

                template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
                {
                    _a._regs.insert(pos);
                }
            };

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                if (_regs.count(pos)) for (unsigned i = 0; i < node.nargs(); ++i) code.pass_temp(handle_src{ *this }, node[i]);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Reg &node) { }
            void operator()(const ir::code &code, ir::word pos, const ir::Arg &node) { }
            void operator()(const ir::code &code, ir::word pos, const ir::Imm &node) { }
            void operator()(const ir::code &code, ir::word pos, const ir::Temp &node) { }
            void operator()(const ir::code &code, ir::word pos, const ir::typecon &node) { }
            void operator()(const ir::code &code, ir::word pos, const ir::Enter &node) { }
            void operator()(const ir::code &code, ir::word pos, const ir::Exit &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Move &node)
            {
                code.pass_temp(handle_src{ *this }, node[1]);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::St &node)
            {
                code.pass_temp(handle_src{ *this }, node[0]);
                code.pass_temp(handle_src{ *this }, node[1]);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Ld &node)
            {
                code.pass_temp(handle_src{ *this }, node[0]);
            }
        };

        template<class OUT, unsigned BITS> struct rtl_gen
        {
            remapper<OUT> _out;
            const rtl_analysis &_a;

            rtl_gen(OUT &out, const rtl_analysis &a) : _out(remapper<OUT>(out)), _a(a) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Reg &node)
            {
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Temp &node)
            {
                _out(pos, node);
            }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                NODE tnode = node;
                for (unsigned i = node.is_write()? 1 : 0; i < node.nargs(); ++i)
                    if (node.is_id(i))
                        if (_a._regs.count(tnode[i]))
                        {
                            ir::word temp = _out.add(ir::Temp(semantics(code, tnode[i]).type().pos()));
                            _out(ir::Move(temp, tnode[i]));
                            tnode[i] = _out.add(ir::Reg(temp, ir::x86::reg_group<BITS>(semantics(code, tnode[i]).type())));
                        }
                        else if (semantics(code, tnode[i]).is<ir::Temp>())
                            tnode[i] = _out.add(ir::Reg(tnode[i], ir::x86::reg_group<BITS>(semantics(code, tnode[i]).type())));
                _out(pos, tnode);
            }
        };

        struct rewriter
        {
            remapper<ir::code> _out;

            rewriter(ir::code &code) : _out(remapper<ir::code>(code)) { }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Mul &node)
            {
                if (semantics(code, node[0]).is<ir::Imm>()) (*this)(code, pos, ir::Mul(node[1], node[0]));
                else if (semantics(code, node[1]).is<ir::Imm>())
                {
                    auto imm = _out.add(ir::Temp(semantics(code, node[0]).type().pos()));
                    _out(ir::Move(imm, node[1]));
                    _out(pos, ir::Mul(node[0], imm));
                }
            }
        };

        template<class OUT, unsigned BITS> OUT rtl(const ir::code &code)
        {
            ir::code rewritten;
            rewriter rwr(rewritten);
            code.pass(rwr);
            rtl_analysis a;
            rewritten.rpass(a);
            OUT out;
            rtl_gen<OUT, BITS> gen(out, a);
            rewritten.pass(gen);
            return out;
        }
    }
}

#endif
