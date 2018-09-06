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

#ifndef CODEGEN_X86_REGS
#define CODEGEN_X86_REGS

#include "x86_ir.h++"

namespace codegen
{
    namespace x86
    {
        class regs64
        {
            std::bitset<16> _iregs;

        public:

            // n should be chosen so that there can never be more than n registers in use at a time
            constexpr static unsigned n = 32;

            regs64()
            {
                reset();
            }

            // Returns a "perfect" register, if available. If not, returns 0.
            ir::word get_free(ir::word id)
            {
                if (ir::x86::is_integer_reg(id))
                {
                    byte index = ir::x86::integer_reg(id).index();
                    if (_iregs[index]) return 0;
                    else
                    {
                        _iregs.set(index);
                        return id;
                    }
                }
                else if (ir::x86::is_int_reg_group(id))
                {
                    if (_iregs.all()) return 0;
                    for (unsigned i = 0; ; ++i) if (!_iregs[i])
                    {
                        _iregs.set(i);
                        return ir::x86::id(integer_reg(ir::x86::log2bits(id), i));
                    }
                }
                // TODO: floats and vectors
            }

            ir::word get_compatible(ir::word id)
            {
                if (ir::word reg = get_free(id)) return reg;
                if (ir::x86::is_integer_reg(id)) return get_free(ir::x86::int_reg_group(id));
                return 0;
            }

            // Returns true if reg is in group. This is used when we need a variable in a register,
            // and it already is in _some_ register. To pick up a register to drop when there is no
            // suitable register available at all, we use is_compatible, which tells us whether a
            // register can be used to store the value at all.
            bool is_perfect(ir::word group, ir::word reg) const
            {
                if (group == reg) return true;
                if (ir::x86::is_integer_reg(reg))
                    return ir::x86::is_int_reg_group(group) && ir::x86::log2bits(reg) == ir::x86::log2bits(group);
                return false;
            }

            bool is_compatible(ir::word group, ir::word reg) const
            {
                if (group == reg) return true;
                if (ir::x86::is_integer_reg(reg)) return ir::x86::is_int_reg_group(group);
                return false;
            }

            void forget(ir::word id)
            {
                if (ir::x86::is_integer_reg(id)) _iregs.reset(ir::x86::integer_reg(id).index());
                // TODO: floats and vectors
            }

            void reset()
            {
                for (unsigned i = 0; i < 16; ++i) _iregs[i] = i == 4 || i == 5;
            }

            template<class GEN> static void remap(GEN &gen, const std::map<ir::word, ir::word> &regs)
            {
                // not very efficient...
                std::map<byte, ir::word> int_regs;
                for (auto r : regs) if (ir::x86::is_integer_reg(r.first))
                    int_regs.emplace(ir::x86::integer_reg(r.first).index(), r.second);
                bool done;
                do
                {
                    done = true;
                    for (auto r = int_regs.begin(); r != int_regs.end(); )
                    {
                        ir::word dst = r->second;
                        auto dstreg = ir::x86::integer_reg(dst);
                        byte index = dstreg.index();
                        if (index == r->first)
                        {
                            ++r;
                            continue;
                        }
                        done = false;
                        if (int_regs.count(index))
                        {
                            ++r;
                            continue;
                        }
                        gen(ir::RMove(r->second, ir::x86::id(integer_reg(dstreg.log2bits(), r->first))));
                        int_regs.emplace(index, dst);
                        r = int_regs.erase(r);
                    }
                }
                while (!done);
                for (auto r : int_regs)
                {
                    auto x = ir::x86::integer_reg(r.second);
                    byte index = x.index();
                    if (index == r.first) continue;
                    auto r2 = int_regs.find(index);
                    gen(ir::RSwap(r2->second, r.second));
                    auto t = r2->second;
                    r2->second = r.second;
                    r.second = t;
                }
            }
        };
    }
}

#endif
