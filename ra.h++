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

#ifndef CODEGEN_RA
#define CODEGEN_RA

#include "remapper.h++"

namespace codegen
{
    template<unsigned N> class rmap
    {
    public:

        struct compact
        {
            ir::word _var[N] = { 0 };
            ir::word _reg[N];
        };

    private:

        ir::word _vars[N] = { 0 };
        unsigned _head = 0;
        std::map<ir::word, ir::word> _regs;

    public:

        rmap() { }

        rmap(const compact &src)
        {
            for (unsigned i = 0; i < N && src._var[i]; ++i) add(src._var[i], src._reg[i]);
        }

        compact compacted()
        {
            compact c;
            for (unsigned i = 0, k = 0; i < N; ++i)
            {
                ir::word var = _vars[(i + _head) % N];
                if (var)
                {
                    c._var[k] = var;
                    c._reg[k++] = _regs[var];
                }
            }
        }

        void add(ir::word var, ir::word reg)
        {
            _vars[_head++] = var;
            _regs[var] = reg;
            if (_head == N) _head = 0;
        }

        void move(ir::word var, ir::word reg)
        {
            _regs[var] = reg;
        }

        void drop(ir::word var)
        {
            _regs.erase(var);
            for (unsigned i = 0; i < N; ++i)
                if (_vars[(_head + i) % N] == var)
                {
                    for (unsigned j = 0; j < i; j++) _vars[(_head + j + 1) % N] = _vars[(_head + j) % N];
                    _vars[_head++] = 0;
                    return;
                }
        }

        bool full() const
        {
            return !!_vars[_head];
        }

        bool contains(ir::word var) const
        {
            return !!_regs.count(var);
        }

        ir::word reg(ir::word var) const
        {
            auto r = _regs.find(var);
            if (r == _regs.end()) return 0;
            return r->second;
        }

        void clear()
        {
            for (unsigned i = 0; i < N; ++i) _vars[i] = 0;
            _regs.clear();
        }

        void combine(const compact &edge)
        {
            for (unsigned i = 0; !full() && edge._var[i]; ++i) add(edge._var[i], edge._reg[i]);
        }

        template<class REGS> void assign(REGS &regs) const
        {
            regs.reset();
            for (auto reg : _regs) regs.get_free(reg.second);
        }

        template<class REGS> ir::word find_perfect(const REGS &regs, ir::word group)
        {
            for (unsigned i = 0; i < N; ++i)
            {
                ir::word var = _vars[(i + _head) % N];
                if (regs.is_perfect(group, _regs[var])) return var;
            }
        }

        template<class REGS> ir::word find_compatible(const REGS &regs, ir::word group)
        {
            for (unsigned i = 0; i < N; ++i)
            {
                ir::word var = _vars[(i + _head) % N];
                if (regs.is_compatible(group, _regs[var])) return var;
            }
        }
    };

    template<class REGS> struct ra
    {
        using regmap = rmap<REGS::n>;

        std::map<ir::word, ir::word> _writes;
        std::map<ir::word, ir::word> _reads;

        struct pass_base
        {
            ra &_ra;

            REGS _regs;
            regmap _map;
            std::unordered_map<ir::word, typename regmap::compact> _edges;

            pass_base(ra &owner) : _ra(owner) { }

            ir::word set_reg(ir::word var, ir::word id)
            {
                ir::word reg = _map.reg(var);
                if (reg)
                {
                    if (_regs.is_perfect(id, reg)) return reg;
                    _regs.forget(reg);
                    _map.drop(var);
                }
                if (reg = _regs.get_free(id))
                {
                    _map.add(var, reg);
                    return reg;
                }
                ir::word to_drop = _map.find_compatible(_regs, id);
                reg = _map.reg(to_drop);
                _regs.forget(reg);
                _map.drop(to_drop);
                if (reg = _regs.get_free(id))
                {
                    _map.add(var, reg);
                    return reg;
                }
                _regs.get_free(reg);
                ir::word to_move = _map.find_perfect(_regs, id);
                ir::word mreg = _map.reg(to_move);
                _map.move(to_move, reg);
                _map.add(var, mreg);
                return mreg;
            }
        };

        struct rev_pass : pass_base
        {
            using pass_base::_ra;
            using pass_base::_regs;
            using pass_base::_map;
            using pass_base::_edges;
            using pass_base::set_reg;

            rev_pass(ra &owner) : pass_base(owner) { }

            void operator()(const ir::code &code, ir::word pos, const ir::node &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Move &node)
            {
                semantics dst(code, node[0]), src(code, node[1]);
                if (dst.is<ir::Temp>())
                {
                    if (ir::word reg = _map.reg(node[0]))
                    {
                        _ra._writes[pos] = reg;
                        if (src.is<ir::Reg>())
                        {
                            _map.drop(node[0]);
                            _map.add(src[0], reg);
                        }
                    }
                }
                else if (dst.is<ir::Reg()>() && src.is<ir::Reg()>())
                {
                    if (ir::word reg = _map.reg(dst[0]))
                    {
                        _ra._writes[pos] = reg;
                        _map.drop(node[0]);
                        _map.add(src[0], reg);
                    }
                }
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Reg &node)
            {
                auto rd = _ra._reads.find(pos);
                if (rd != _ra._reads.end()) set_reg(node[0], rd->second);
                else set_reg(node[0], node[1]);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Here &node)
            {
                _edges[node[0]] = _map.compacted();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Skip &node)
            {
                _map = regmap(_edges[pos]);
                _map.assign(_regs);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::SkipIf &node)
            {
                _regs.reset();
                _map.combine(_edges[pos]);
                _map.assign(_regs);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Forever &node)
            {
                _edges[pos] = _map.compacted();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                auto edge = _edges.find(node[0]);
                if (edge != _edges.end())
                {
                    _regs.reset();
                    _map = regmap(edge->second);
                    _map.assign(_regs);
                }
                else
                {
                    _regs.reset();
                    _map.clear();
                }
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Exit &node)
            {
                _regs.reset();
                _map.clear();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Enter &node)
            {
                _regs.reset();
                _map.clear();
            }
        };

        struct fwd_pass : pass_base
        {
            using pass_base::_ra;
            using pass_base::_regs;
            using pass_base::_map;
            using pass_base::_edges;
            using pass_base::set_reg;

            fwd_pass(ra &owner) : pass_base(owner) { }

            void operator()(const ir::code &code, ir::word pos, const ir::node &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::Move &node)
            {
                semantics dst(code, node[0]);
                auto wr = _ra._writes.find(pos);
                if (dst.is<ir::Reg>()) set_reg(dst[0], dst[1]);
                else if (wr != _ra._writes.end())
                    if (ir::word reg = _regs.get_compatible(wr->second))
                    {
                        _map.add(dst.pos(), reg);
                        wr->second = reg;
                    }
                    else set_reg(dst.pos(), wr->second);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Reg &node)
            {
                if (ir::word reg = _map.reg(node[0]))
                    if (_regs.is_perfect(node[1], reg)) _ra._reads[pos] = reg;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Here &node)
            {
                semantics skip(code, node[0]);
                _regs.reset();
                if (skip.is<ir::SkipIf>()) _map.combine(_edges[node[0]]);
                else _map = regmap(_edges[node[0]]);
                _map.assign(_regs);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Skip &node)
            {
                _edges[pos] = _map.compacted();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::SkipIf &node)
            {
                _edges[pos] = _map.compacted();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Forever &node)
            {
                regmap temp(_edges[pos]);
                temp.combine(_map.compacted());
                _regs.reset();
                _map = temp;
                _map.assign(_regs);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                _edges[node[0]] = _map.compacted();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Exit &node)
            {
                _regs.reset();
                _map.clear();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Enter &node)
            {
                _regs.reset();
                _map.clear();
            }
        };

        template<class OUT> struct gen_pass
        {
            fwd_pass _fwd;
            remapper<OUT> _out;

            gen_pass(OUT &out, fwd_pass &fwd) : _fwd(fwd), _out(remapper<OUT>(out)) { }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                semantics sem(code, pos);
                if (sem.is<ir::wrnode>())
                {
                    NODE tnode = node;
                    auto wr = _fwd._ra._writes.find(pos);
                    if (wr != _fwd._ra._writes.end())
                    {
                        tnode[0] = _out.add(ir::Reg(node[0], wr->second));
                        _out(pos, tnode);
                        return;
                    }
                }
                _fwd(code, pos, node);
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Reg &node)
            {
                auto rd = _fwd._ra._reads.find(pos);
                if (rd != _fwd._ra._reads.end()) _out(pos, ir::Reg(node[0], rd->second));
                else _out(pos, node);
            }
        };

        template<class OUT> void process(OUT &out, const ir::code &code, unsigned n)
        {
            rev_pass rev(*this);
            fwd_pass fwd(*this);
            while (n--)
            {
                code.rpass(rev);
                code.pass(fwd);
            }
            code.rpass(rev);
            gen_pass<OUT> gen(out, fwd);
            code.pass(gen);
        }
    };
}

#endif
