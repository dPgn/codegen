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

#ifndef CODEGEN_H
#define CODEGEN_H

#include "x86_gen.h++"
#include "x86_rtl.h++"
#include "x86_regs.h++"
#include "x86_cc.h++"

#include "control.h++"
#include "ra.h++"

namespace codegen
{
    template<class FUN> function<FUN> build(ir::code &code)
    {
        ir::code pre_ra;
        x86::cc<ir::code>::pre_ra_gen pre_ra_gen(pre_ra);
        code.pass(pre_ra_gen);
        ir::code rtl = x86::rtl<ir::code, 64>(pre_ra);
        ir::code post_ra;
        ra<x86::regs64>().process(post_ra, rtl, 1);
        x86::function_gen<FUN> gen;
        post_ra.pass(gen);
        return gen.fun();
    }
}

#endif
