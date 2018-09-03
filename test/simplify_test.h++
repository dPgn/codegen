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

TEST(Simplify, Basic)
{
    std::string unsimplified =
        "[ fun: Enter [ Fun 0 [ Int -32 ] ] ]"
        "[ a: Temp [ Int -32] ]"
        "[ b: Temp [ Int -32] ]"
        "[ c: Temp [ Int -32] ]"
        "[ Move a [ Mul [ Cast [ Int -32 ] [ 2 ] ] [ 3 ] ] ]"
        "[ s0: SkipIf [ Gt a [ 5 ] ] ]"
        "[ Move b [ 4 ] ]"
        "[ s1: Skip ]"
        "[ Here s0 ]"
        "[ Move b a ]"
        "[ Here s1 ]"
        "[ Move c [ Sub [ Cast [ Int -32 ] [ 13 ] ] b ] ]"
        "[ Move [ RVal fun ] [ Mul b c ] ]"
        "[ Exit fun ]";

    std::string simplified =
        "[ fun: Enter [ Fun 0 [ Int -32 ] ] ]"
        "[ Move [ RVal fun ] [ Cast [ Int -32 ] [ 42 ] ] ]"
        "[ Exit fun ]";

    std::string result = simplify(parse(unsimplified), 10).text();
    std::string reference = parse(simplified).text();

//    std::cout << reference << std::endl;
//    std::cout << result << std::endl;

    ASSERT_EQ(reference, result);
}
