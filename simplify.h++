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
// elimination, and common subexpression elimination.

namespace codegen
{
    class simplifier
    {
        struct fwd_pass
        {
        };

        struct rev_pass
        {
        };
    };
}

#endif
