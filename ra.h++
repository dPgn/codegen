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

        template<class REGS> ir::word find_perfect(const REGS &regs, ir::word group) const
        {
            for (unsigned i = 0; i < N; ++i)
            {
                ir::word var = _vars[(i + _head) % N];
                if (var && regs.is_perfect(group, _regs.at(var))) return var;
            }
            return 0;
        }

        template<class REGS> ir::word find_compatible(const REGS &regs, ir::word group) const
        {
            for (unsigned i = 0; i < N; ++i)
            {
                ir::word var = _vars[(i + _head) % N];
                if (var && regs.is_compatible(group, _regs.at(var))) return var;
            }
            return 0;
        }

        template<class ACTION> void change_from(const rmap &other, ACTION action) const
        {
            std::map<ir::word, ir::word> common;

            for (unsigned i = 0; i < N; ++i)
                if (ir::word var = other._vars[i])
                    if (!contains(var)) action.spill(other.reg(var), var);
                    else common.emplace(other.reg(var), reg(var));

            action.remap(common);

            for (unsigned i = 0; i < N; ++i)
                if (ir::word var = _vars[i])
                    if (other.contains(var)) action.fill(reg(var), var);
        }
    };

    template<class REGS> struct ra
    {
        using regmap = rmap<REGS::n>;

        std::map<ir::word, ir::word> _writes;
        std::map<ir::word, ir::word> _reads;

        struct pass_base
        {
            struct do_nothing
            {
                void move(ir::word dst, ir::word src) { }
                void spill(ir::word reg, ir::word var) { }
            };

            ra &_ra;

            REGS _regs;
            regmap _map;
            std::unordered_map<ir::word, typename regmap::compact> _edges;

            pass_base(ra &owner) : _ra(owner) { }

            template<class ACTION = do_nothing> ir::word set_reg(ir::word var, ir::word id, ACTION action = do_nothing())
            {
                ir::word reg = _map.reg(var), freg, dreg;
                if (reg)
                {
                    if (_regs.is_perfect(id, reg)) return reg;
                    _regs.forget(reg);
                    _map.drop(var);
                }
                if (freg = _regs.get_free(id))
                {
                    _map.add(var, freg);
                    if (reg) action.move(freg, reg);
                    return freg;
                }
                ir::word to_drop = _map.find_compatible(_regs, id);
                dreg = _map.reg(to_drop);
                _regs.forget(dreg);
                _map.drop(to_drop);
                action.spill(dreg, to_drop);
                if (freg = _regs.get_free(id))
                {
                    _map.add(var, freg);
                    if (reg) action.move(freg, reg);
                    return freg;
                }
                _regs.get_free(dreg);
                ir::word to_move = _map.find_perfect(_regs, id);
                ir::word mreg = _map.reg(to_move);
                _map.move(to_move, dreg);
                action.move(dreg, mreg);
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

            std::unordered_map<ir::word, unsigned> _loop_level;
            unsigned _level;

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
                if (_loop_level[pos] > _level) _map.combine(_edges[pos]);
                else
                {
                    regmap temp = regmap(_edges[pos]);
                    temp.combine(_map.compacted());
                    _map = temp;
                }
                _map.assign(_regs);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Forever &node)
            {
                --_level;
                _edges[pos] = _map.compacted();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                ++_level;
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

        struct loop_level_counter
        {
            rev_pass &_rev;

            loop_level_counter(rev_pass &rev) : _rev(rev) { }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                _rev(code, pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Here &node)
            {
                _rev._loop_level[node[0]] = _rev._level;
            }
        };

        struct fwd_pass : pass_base
        {
            using pass_base::_ra;
            using pass_base::_regs;
            using pass_base::_map;
            using pass_base::_edges;
            using pass_base::set_reg;

            rev_pass &_rev;
            unsigned _level = 0;

            fwd_pass(ra &owner, rev_pass &rev) : pass_base(owner), _rev(rev) { }

            void operator()(const ir::code &code, ir::word pos, const ir::node &node) { }

            void operator()(const ir::code &code, ir::word pos, const ir::wrnode &node)
            {
                semantics dst(code, semantics(code, pos)[0]);
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
                ++_level;
                regmap temp(_edges[pos]);
                temp.combine(_map.compacted());
                _regs.reset();
                _map = temp;
                _map.assign(_regs);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                --_level;
                _edges[node[0]] = _map.compacted();
                _regs.reset();
                _map.clear();
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
            struct action
            {
                gen_pass &_gen;

                action(gen_pass &gen) : _gen(gen) { }

                void remap(const std::map<ir::word, ir::word> &common)
                {
                    _gen._fwd._regs.remap(_gen._out, common);
                }

                void spill(ir::word reg, ir::word var)
                {
                    _gen._out(ir::Move(var, _gen._out.add(ir::Reg(var, reg))));
                }

                void fill(ir::word reg, ir::word var)
                {
                    _gen._out(ir::Move(_gen._out.add(ir::Reg(var, reg)), var));
                }

                void move(ir::word dst, ir::word src)
                {
                    _gen._out(ir::RMove(dst, src));
                }
            };

            fwd_pass &_fwd;
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

            void operator()(const ir::code &code, ir::word pos, const ir::wrnode &node)
            {
                semantics dst(code, semantics(code, pos)[0]);
                auto wr = _fwd._ra._writes.find(pos);
                if (dst.is<ir::Reg>()) _fwd.set_reg(dst[0], dst[1], action(*this));
                else if (wr != _fwd._ra._writes.end())
                    if (ir::word reg = _fwd._regs.get_compatible(wr->second))
                    {
                        _fwd._map.add(dst.pos(), reg);
                        wr->second = reg;
                    }
                    else _fwd.set_reg(dst.pos(), wr->second, action(*this));
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Reg &node)
            {
                auto rd = _fwd._ra._reads.find(pos);
                if (rd != _fwd._ra._reads.end()) _out(pos, ir::Reg(node[0], rd->second));
                else _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Skip &node)
            {
                _fwd._edges[pos] = _fwd._map.compacted();
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::SkipIf &node)
            {
                /*
                if (_fwd._rev._loop_level[pos] <= _fwd._level)
                    _fwd._edges[pos] = _fwd._map.compacted();
                else
                {
                    _fwd._regs.reset();
                    regmap omap = _fwd._map;
                    _fwd._map = regmap(_fwd._edges[pos] = _fwd._rev._edges[pos]);
                    _fwd._map.change_from(omap, action(*this));
                    _fwd._map.assign(_fwd._regs);
                }
                */
                // TODO: It is entirely possible that the above register shuffling invalidated the
                // condition of our SkipIf, as the nodes that actually compute the condition come
                // before the SkipIf node, and the spill/move/fill code is inserted _immediately_
                // before the SkipIf. This is a critical problem that can produce invalid results
                // and should, therefore, be fixed ASAP. For now, just do this suboptimally:
                _fwd._edges[pos] = _fwd._map.compacted(); // fix the above and remove this!!!
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Here &node)
            {
                semantics skip(code, node[0]);
                _fwd._regs.reset();
                regmap omap = _fwd._map;
                _fwd._map = regmap(_fwd._edges[node[0]]);
                _fwd._map.change_from(omap, action(*this));
                _fwd._map.assign(_fwd._regs);
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Forever &node)
            {
                // TODO: moves, fills, spills
                ++_fwd._level;
                regmap temp(_fwd._edges[pos]);
                temp.combine(_fwd._map.compacted());
                _fwd._regs.reset();
                temp.change_from(_fwd._map, action(*this));
                _fwd._map = temp;
                _fwd._map.assign(_fwd._regs);
                _out(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                --_fwd._level;
                regmap(_fwd._edges[pos]).change_from(_fwd._map, action(*this));
                _fwd._map.clear();
                _fwd._regs.reset();
                _out(pos, node);
                // Nothing is saved here, because the generator pass is the last one by definition.
            }
        };

        template<class OUT> void process(OUT &out, const ir::code &code, unsigned n)
        {
            rev_pass rev(*this);
            fwd_pass fwd(*this, rev);
            loop_level_counter llc(rev);
            code.rpass(llc);
            while (n--)
            {
                code.pass(fwd);
                code.rpass(rev);
            }
            gen_pass<OUT> gen(out, fwd);
            code.pass(gen);
        }
    };
}

#endif
