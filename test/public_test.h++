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

TEST(Public, AdditionDSL)
{
    ir::code code;

    auto i32        = code(ir::Int(-32));
    auto funtype    = code(ir::Fun(0, i32, i32, i32));
    auto fun        = code(ir::Enter(funtype));
    auto arg0       = code(ir::Arg(fun, 0));
    auto arg1       = code(ir::Arg(fun, 1));
    auto rval       = code(ir::RVal(fun));
    auto sum        = code(ir::Add(arg0, arg1));

    code(ir::Move(rval, sum));
    code(ir::Exit(fun));

    auto add_two_numbers = build<std::int_least32_t(std::int_least32_t, std::int_least32_t)>(code);

    ASSERT_EQ(42, add_two_numbers(19, 23));
}

TEST(Public, AdditionTextual)
{
    std::string codetext =
        "[ fun: Enter [ Fun 0 [ Int -32 ] [ Int -32 ] [ Int -32] ] ]"
        "[ Move [ RVal fun ] [ Add [ Arg fun 0 ] [ Arg fun 1 ] ] ]"
        "[ Exit fun ]";

    ir::code code = textual(codetext).code();

    auto add_two_numbers = build<std::int_least32_t(std::int_least32_t, std::int_least32_t)>(code);

    ASSERT_EQ(42, add_two_numbers(19, 23));
}
