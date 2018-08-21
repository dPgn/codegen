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

TEST(Control, Structurize)
{
    std::string unstructured =
        "[ i64: Int -64 ]"
        "[ fun: Enter [ Fun 0 i64 i64 i64 ] ]"
        "[ a: Label ] [ Mark a ]"
        "[ Move [ RVal fun ] [ Arg fun 0 ] ]" // just to do something
        "[ b: Label ] [ Mark b ]"
        "[ Branch a [ Gt [ Arg fun 1 ] [ Arg fun 0 ] ] ]"
        "[ Move [ RVal fun ] [ Arg fun 1 ] ]" // again just something
        "[ Branch b [ Eq [ Arg fun 1 ] [ Arg fun 0 ] ] ]"
        "[ Jump a ]"
        "[ Exit fun ]";

    // The above should be something like this when the control flow is structurized.
    std::string structured =
        "[ i64: Int -64 ]"
        "[ fun: Enter [ Fun 0 i64 i64 i64 ] ]"
        "[ a: Forever ]"
        "[ Move [ RVal fun ] [ Arg fun 0 ] ]"
        "[ b: Forever ]"
        "[ c: SkipIf [ Not [ Gt [ Arg fun 1 ] [ Arg fun 0 ] ] ] ]"
        "[ d: Skip ]"
        "[ Here c ]"
        "[ Move [ RVal fun ] [ Arg fun 1 ] ]"
        "[ e: SkipIf [ Not [ Eq [ Arg fun 1 ] [ Arg fun 0 ] ] ] ]"
        "[ Repeat b ]"
        "[ Here e ]"
        "[ Here d ]"
        "[ Repeat a ]"
        "[ Exit fun ]";

    ASSERT_EQ(control::structurized<ir::code>(textual(unstructured).code()).text(), textual(structured).code().text());
}

TEST(Control, Unstructurize)
{
    // Copy-pasted from above
    std::string structured =
        "[ i64: Int -64 ]"
        "[ fun: Enter [ Fun 0 i64 i64 i64 ] ]"
        "[ a: Forever ]"
        "[ Move [ RVal fun ] [ Arg fun 0 ] ]"
        "[ b: Forever ]"
        "[ c: SkipIf [ Not [ Gt [ Arg fun 1 ] [ Arg fun 0 ] ] ] ]"
        "[ d: Skip ]"
        "[ Here c ]"
        "[ Move [ RVal fun ] [ Arg fun 1 ] ]"
        "[ e: SkipIf [ Not [ Eq [ Arg fun 1 ] [ Arg fun 0 ] ] ] ]"
        "[ Repeat b ]"
        "[ Here e ]"
        "[ Here d ]"
        "[ Repeat a ]"
        "[ Exit fun ]";

    std::string unstructured =
        "[ i64: Int -64 ]"
        "[ fun: Enter [ Fun 0 i64 i64 i64 ] ]"
        "[ a: Label ] [ Mark a ]"
        "[ Move [ RVal fun ] [ Arg fun 0 ] ]"
        "[ b: Label ] [ Mark b ]"
        "[ c: Label ] [ Branch c [ Not [ Gt [ Arg fun 1 ] [ Arg fun 0 ] ] ] ]"
        "[ d: Label ] [ Jump d ]"
        "[ Mark c ]"
        "[ Move [ RVal fun ] [ Arg fun 1 ] ]"
        "[ e: Label ] [ Branch e [ Not [ Eq [ Arg fun 1 ] [ Arg fun 0 ] ] ] ]"
        "[ Jump b ]"
        "[ Mark e ]"
        "[ Mark d ]"
        "[ Jump a ]"
        "[ Exit fun ]";

    ASSERT_EQ(control::unstructurized<ir::code>(textual(structured).code()).text(), textual(unstructured).code().text());
}
