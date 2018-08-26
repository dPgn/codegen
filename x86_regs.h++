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
        };
    }
}

#endif
