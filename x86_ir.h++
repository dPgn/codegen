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

#ifndef CODEGEN_X86_IR_H
#define CODEGEN_X86_IR_H

#include "ir.h++"
#include "x86_asm.h++"

namespace codegen
{
    namespace ir
    {
        namespace x86
        {
            namespace x86 = ::codegen::x86;

            static constexpr ir::word id(const x86::integer_reg &r)
            {
                return -(r.index() | (r.log2bits() << 5));
            }

            static constexpr bool is_integer_reg(ir::word id)
            {
                id = -id;
                return id >= (3 << 5) && id < ((6 << 5) | 0x16); // TODO: filter out all with bit 4 set apart from 8 bit high half regs
            }

            static constexpr x86::integer_reg integer_reg(ir::word id)
            {
                id = -id;
                return x86::integer_reg(id >> 5, id & 0x1f);
            }
        }
    }
}

#endif
