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

// This is the test program for my codegen library. It is purposefully entirely in one source file;
// if it gets too big to compile in tolerable time, that is a sign that the library has grown too
// big and should be split into multiple porjects, each with its own test program.
//
// This test program is meant to act not just as a series of unit tests but also – along with its
// comments – as a definitive specification and reference, as well as example code for every
// feature of the library. While not convenient as a user manual, it should be clear, complete,
// and unambiguous enough to be everything needed to write one. I might test this by finding
// someone else to do that without my help or reading the code of the actual library.

#include <gtest/gtest.h>

#include "../codegen.h++"

using namespace codegen;

#include "ir_test.h++"

#include "x86_asm_test.h++"
#include "x86_gen_test.h++"

#include "textual_test.h++"

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
