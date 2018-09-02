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

#ifndef CODEGEN_X86_CC_H
#define CODEGEN_X86_CC_H

#include "x86_asm.h++"
#include "x86_ir.h++"
#include "semantics.h++"
#include "remapper.h++"

namespace codegen
{
    namespace x86
    {
        template<class OUT> struct cc
        {
            struct conv_gen
            {
                virtual void arg(unsigned k, const semantics &ty) = 0;
                virtual void rval(const semantics &type) = 0;

                virtual void operator()(const ir::code &code, ir::word pos, const ir::Exit &node) = 0;
                virtual void operator()(const ir::code &code, ir::word pos, const ir::RVal &node) = 0;
                virtual void operator()(const ir::code &code, ir::word pos, const ir::Arg &node) = 0;
            };

            struct funtype
            {
                conv_gen &_gen;

                funtype(conv_gen &gen) : _gen(gen) { }

                void operator()(const ir::code &code, ir::word pos, const ir::node &node) { }

                void operator()(const ir::code &code, ir::word pos, const ir::Fun &node)
                {
                    for (unsigned i = 2; i < node.nargs(); ++i)
                    {
                        _gen.arg(i - 2, semantics(code, node[i]));
                    }
                    _gen.rval(semantics(code, node[1]));
                }
            };

            struct system_v_64
            {
                struct pre_ra_gen : conv_gen
                {
                    ir::word _rval;
                    ir::word _rty;
                    std::vector<ir::word> _args;

                    ir::word _fun;
                    remapper<OUT> &_out;
                    unsigned _nintargs = 0;

                    pre_ra_gen(remapper<OUT> &out, ir::word fun) : _out(out), _fun(fun) { }

                    static constexpr byte arg_reg(unsigned k)
                    {
                        // rdi, rsi, rdx, rcx, r8, r9
                        // = 7, 6, 2, 1, 8, 9
                        return k < 2? 7 - k : k < 4? 4 - (k & 1) : k + 4;
                    }

                    void arg(unsigned k, const semantics &ty)
                    {
                        auto temp = _out.add(ir::Temp(ty.pos()));
                        _args.push_back(temp);
                        if ((ty.is<ir::Int>() || ty.is<ir::Ptr>()) && (_nintargs++ < 6))
                        {
                            auto b = std::abs(ty[0]);
                            byte log2bits = ty.is<ir::Ptr>()? 6 : b > 32? 6 : b > 16? 5 : b > 8? 4 : 3;
                            _out(ir::Move(_out.add(ir::Reg(temp, ir::x86::id(integer_reg(log2bits, arg_reg(_nintargs - 1))))), _out.add(ir::Arg(_fun, k))));
                            return;
                        }
                        // TODO: floats, vectors, small structs

                        // If it's not in a register, it's just a temporary pre-ra:
                        _out(ir::Move(_out.add(ir::Temp(ty.pos())), _out.add(ir::Arg(_fun, k))));
                    }

                    void rval(const semantics &type)
                    {
                        _rty = type.pos();
                        _rval = _out.add(ir::Temp(type.pos()));
                    }

                    void operator()(const ir::code &code, ir::word pos, const ir::Exit &node)
                    {
                        auto rval = _rval;
                        auto ty = semantics(code, _rty);
                        if (ty.is<ir::Int>() || ty.is<ir::Ptr>())
                        {
                            auto b = std::abs(ty[0]);
                            byte log2bits = ty.is<ir::Ptr>()? 6 : b > 32? 6 : b > 16? 5 : b > 8? 4 : 3;
                            _out(ir::Move(_out.add(ir::RVal(node[0])), _out.add(ir::Reg(rval, ir::x86::id(integer_reg(log2bits, 0))))));
                        }
                        _out(node);
                    }

                    void operator()(const ir::code &code, ir::word pos, const ir::Arg &node)
                    {
                        _out.map(_args[node[1]], pos);
                    }

                    void operator()(const ir::code &code, ir::word pos, const ir::RVal &node)
                    {
                        _out.map(_rval, pos);
                    }
                };
            };

            struct pre_ra_gen
            {
                remapper<OUT> _out;

                conv_gen *_gen = nullptr;

                pre_ra_gen(OUT &out) : _out(out) { }

                template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
                {
                    _out(pos, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Enter &node)
                {
                    _out(pos, node);
                    // This will be used to choose between calling conventions once we support more than one
                    ir::word cc_type = semantics(code, node[0])[0];
                    _gen = new typename system_v_64::pre_ra_gen(_out, pos);
                    code.pass_temp(funtype(*_gen), node[0]);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Exit &node)
                {
                    (*_gen)(code, pos, node);
                    delete _gen;
                    _gen = nullptr;
                }

                void operator()(const ir::code &code, ir::word pos, const ir::Arg &node)
                {
                    (*_gen)(code, pos, node);
                }

                void operator()(const ir::code &code, ir::word pos, const ir::RVal &node)
                {
                    (*_gen)(code, pos, node);
                }
            };
        };
    }
}

#endif
