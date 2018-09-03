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

#ifndef CODEGEN_SIMPLIFY_H
#define CODEGEN_SIMPLIFY_H

#include "semantics.h++"

// So here we simplify our IR code, much in the same way as we'd simplify an arithmetic expression.
// Or, in a sense, _exactly_ the same way; our axioms are just very different. In traditional
// compiler backend terminology, my "simplification" translates to constant propagation, dead code
// elimination, and common subexpression elimination. For now.

// For effective common subexpression elimination, we should remove all Temp variables that are
// only assigned once, replacing their uses with the value assigned to the variable. This is
// effectively a poor man's SSA form, without all the trouble with the strict, proper SSA form. It
// may not be legal IR, because the uses can be in different basic blocks, but we can restore the
// Temp variables later after simplification, and the simplifier itself can be written to take this
// relaxed rule into account. Some more advanced tricks could improve subexpression elimination
// further, but this is a simple, low-effort approach that has the potential to improve real-world
// code significantly.

namespace codegen
{
    namespace simplifier
    {
        struct rev_pass
        {
            struct uses_query
            {
                using rval = bool;

                bool operator()(const ir::code &code, ir::word pos, const ir::node &node) const
                {
                    return true;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::arithmetic &node) const
                {
                    return false;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::compare &node) const
                {
                    return false;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::Cast &node) const
                {
                    return false;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::Conv &node) const
                {
                    return false;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::Temp &node) const
                {
                    return false;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::Imm &node) const
                {
                    return false;
                }

                bool operator()(const ir::code &code, ir::word pos, const ir::typecon &node) const
                {
                    return false;
                }
            };

            std::vector<bool> _used;

            rev_pass(std::size_t size) : _used(size, false) { }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                if (_used[pos] = (_used[pos] | code.query_at(uses_query(), pos)))
                {
                    if (semantics(code, pos).is<ir::wrnode>())
                        if (_used[node[0] + 1]) _used[node[0] + 2] = true;
                        else _used[node[0] + 1] = true;
                    for (unsigned i = 0; i < node.nargs(); ++i) if (node.is_id(i)) _used[node[i]] = true;
                }
            }
        };

        struct fwd_pass
        {
            struct key
            {
                ir::word _op, _x, _y;

                bool operator<(const key &other) const
                {
                    return _op < other._op || _op == other._op && (_x < other._x || _x == other._x && _y < other._y);
                }

                bool operator==(const key &other) const
                {
                    return _op == other._op && _x == other._x && _y == other._y;
                }
            };

            struct here_query
            {
                using rval = ir::word;

                rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
                {
                    return 0;
                }

                rval operator()(const ir::code &code, ir::word pos, const ir::Here &node) const
                {
                    return node[0];
                }
            };

            struct constant_propagator
            {
                fwd_pass &_fwd;

                constant_propagator(fwd_pass &fwd) : _fwd(fwd) { }

                struct int_calculator
                {
                    using rval = ir::word;

                    ir::word _x, _y;

                    rval operator()(const ir::code &code, ir::word pos, const ir::node &node) const
                    {
                        throw 0; // TODO: something smarter
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Add &node) const
                    {
                        return _x + _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Sub &node) const
                    {
                        return _x - _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Mul &node) const
                    {
                        return _x * _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Div &node) const
                    {
                        if (!semantics(code, pos).is_signed())
                        {
                            std::uint_least64_t x = _x, y = _y;
                            return x / y;
                        }
                        return _x / _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::And &node) const
                    {
                        return _x & _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Or &node) const
                    {
                        return _x | _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Xor &node) const
                    {
                        return _x ^ _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Eq &node) const
                    {
                        return _x == _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Neq &node) const
                    {
                        return _x != _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Gt &node) const
                    {
                        return _x > _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Gte &node) const
                    {
                        return _x >= _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Lt &node) const
                    {
                        return _x < _y;
                    }

                    rval operator()(const ir::code &code, ir::word pos, const ir::Lte &node) const
                    {
                        return _x <= _y;
                    }
                };

                std::map<ir::word, ir::word> _int_constants;

                ir::word _last_pos;

                template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
                {
                    auto ty = semantics(code, pos).type();
                    auto ir = _int_constants.find(pos);
                    if (ir != _int_constants.end())
                    {
                        (*_fwd._out)(pos, ir::Cast(ty.pos(), _fwd._out->add(ir::Imm(ir->second))));
                        _last_pos = pos;
                        return;
                    }
                    auto ix = _int_constants.find(node[0]);
                    if (ix != _int_constants.end())
                    {
                        auto iy = _int_constants.find(node[1]);
                        if (iy != _int_constants.end())
                        {
                            ir::word r = _int_constants[pos] = code.query_at(int_calculator{ ix->second, iy->second }, pos);
                            /* if (ty.is_bool()) (*_fwd._out)(pos, ir::Imm(r));
                            else */ (*_fwd._out)(pos, ir::Cast(ty.pos(), _fwd._out->add(ir::Imm(r))));
                            _last_pos = pos;
                            return;
                        }
                    }
                }

                void clear()
                {
                    _int_constants.clear();
                }
            };

            remapper<ir::code> *_out;
            rev_pass *_rev;
            std::map<key, ir::word> _subexpr;
            std::map<ir::word, ir::word> _argmap;
            constant_propagator _constants;
            std::set<ir::word> _deleted_skips, _deleted_forevers;
            bool _inhibit = false;

            fwd_pass() : _constants(*this) { }

            void reset()
            {
                _constants.clear();
                _subexpr.clear();
                _argmap.clear();
                _inhibit = false;
            }

            ir::word find_arg(ir::word arg)
            {
                auto ai = _argmap.find(arg);
                return ai == _argmap.end()? arg : ai->second;
            }

            // We handle all (both) unary operations specifically, so all that comes here is binary.
            template<class NODE> void do_arithmetic(const ir::code &code, ir::word pos, const NODE &node)
            {
                code.pass_at(_constants, pos);
                if (_constants._last_pos == pos) return;
                ir::word x = find_arg(node[0]), y = find_arg(node[1]);
                key k{ node.id(), x, y };
                auto si = _subexpr.find(k);
                if (si != _subexpr.end())
                {
                    _out->map(pos, si->second);
                    _argmap[pos] = si->second;
                }
                else
                {
                    code.forward(*_out, pos);
                    _subexpr[k] = pos;
                }
            }

            template<class NODE> void operator()(const ir::code &code, ir::word pos, const NODE &node)
            {
                if (!_rev->_used[pos]) return;
                semantics sem(code, pos);
                if (sem.is<ir::arithmetic>() || sem.is<ir::compare>()) do_arithmetic(code, pos, node);
                else (*_out)(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Temp &node)
            {
                if (!_rev->_used[pos]) return;
                (*_out)(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Move &node)
            {
                if (_inhibit) return;
                if (semantics(code, node[0]).is<ir::Temp>() && !_rev->_used[node[0]]) return;
                if (semantics(code, node[0]).is<ir::Temp>() && !_rev->_used[node[0] + 2]) _out->map(node[1], node[0]);
                else (*_out)(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Imm &node)
            {
                if (!_rev->_used[pos]) return;
                (*_out)(pos, node);
                _constants._int_constants[pos] = node[0];
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Cast &node)
            {
                if (!_rev->_used[pos]) return;
                (*_out)(pos, node);
                auto ci = _constants._int_constants.find(node[1]);
                if (ci != _constants._int_constants.end()) _constants._int_constants[pos] = ci->second;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Conv &node)
            {
                if (!_rev->_used[pos]) return;
                (*_out)(pos, node);
                auto ci = _constants._int_constants.find(node[1]);
                if (ci != _constants._int_constants.end()) _constants._int_constants[pos] = ci->second;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Not &node)
            {
                if (!_rev->_used[pos]) return;
                semantics src(code, node[0]);
                if (src.is<ir::Not>()) _out->map(pos, src[0]);
                else (*_out)(pos, node);
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Neg &node)
            {
                if (!_rev->_used[pos]) return;
                semantics src(code, node[0]);
                if (src.is<ir::Neg>()) _out->map(pos, src[0]);
                else (*_out)(pos, node);
            }

            void basic_block_changed()
            {
                // TODO: Handle single assignment temporaries somehow
                _argmap.clear();
                _subexpr.clear();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Skip &node)
            {
                if (_inhibit)
                {
                    _deleted_skips.insert(pos);
                    return;
                }
                ir::word tpos = pos, skip;
                while (code.next(tpos) && (skip = code.query_at(here_query(), tpos))) if (skip == pos)
                {
                    _deleted_skips.insert(pos);
                    return;
                }
                (*_out)(pos, node);
                basic_block_changed();
                _inhibit = true;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::SkipIf &node)
            {
                if (_inhibit)
                {
                    _deleted_skips.insert(pos);
                    return;
                }
                ir::word tpos = pos, skip;
                while (code.next(tpos) && (skip = code.query_at(here_query(), tpos))) if (skip == pos)
                {
                    _deleted_skips.insert(pos);
                    return;
                }
                auto ci = _constants._int_constants.find(node[0]);
                if (ci != _constants._int_constants.end())
                {
                    if (ci->second) (*this)(code, pos, ir::Skip());
                    else _deleted_skips.insert(pos);
                    return;
                }
                (*_out)(pos, node);
                basic_block_changed();
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Here &node)
            {
                auto ds = _deleted_skips.find(node[0]);
                if (ds != _deleted_skips.end())
                {
                    _deleted_skips.erase(ds);
                    return;
                }
                (*_out)(pos, node);
                basic_block_changed();
                _inhibit = false;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Forever &node)
            {
                if (_deleted_forevers.count(pos)) return;
                (*_out)(pos, node);
                basic_block_changed();
                _inhibit = false;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Repeat &node)
            {
                auto dr = _deleted_forevers.find(node[0]);
                if (dr != _deleted_forevers.end())
                {
                    _deleted_forevers.erase(dr);
                    _inhibit = true;
                    return;
                }
                if (_inhibit)
                {
                    _deleted_forevers.insert(node[0]);
                    return;
                }
                (*_out)(pos, node);
                basic_block_changed();
                _inhibit = true;
            }

            void operator()(const ir::code &code, ir::word pos, const ir::Exit &node)
            {
                (*_out)(pos, node);
                // None of our common subexpressions survive past the end of a function
                _argmap.clear();
                _subexpr.clear();
            }
        };
    }

    static ir::code simplify(const ir::code &code, unsigned n)
    {
        ir::code c[2];
        remapper<ir::code> rc[2] { c[0], c[1] };
        simplifier::rev_pass rev(code.size());
        simplifier::fwd_pass fwd;
        code.rpass(rev);
        fwd._rev = &rev;
        fwd._out = &rc[0];
        code.pass(fwd);
        fwd.reset();
        for (unsigned i = 0; i < n; ++i)
        {
            rev = simplifier::rev_pass(c[i & 1].size());
            c[i & 1].rpass(rev);
            fwd._rev = &rev;
            rc[~i & 1].reset();
            fwd._out = &rc[~i & 1];
            c[i & 1].pass(fwd);
            fwd.reset();
        }
        return c[n & 1];
    }
}

#endif
