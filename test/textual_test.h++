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

#include "../textual.h++"

TEST(Textual, Minimal)
{
    textual txt("[i64: Int -64][fun: Enter [Fun 0 i64 i64 i64]][Exit fun]");
    std::string s = txt.code().text();
    textual txt2(s);
    ASSERT_EQ(s, txt2.code().text());
}
