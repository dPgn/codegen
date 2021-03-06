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

TEST(IR, RawBuffer)
{
    ir::buffer buf;

    // The intermediate representation is stored in a buffer that, in its core, stores raw 64 bit
    // signed integers, with smaller magnitude numbers taking less space. This flat structure is
    // meant to improve locality and reduce memory footprint.
    buf.write(0);
    buf.write(42);
    buf.write(-666);
    buf.write(0x12345689abcdefLL);
    buf.write(0x77777777777777LL);
    buf.write(-0x66666666666666LL);
    buf.write(13);

    // The iterators are not quite identical to STL but close enough to justify the same naming
    auto i = buf.begin();
    ASSERT_EQ(i, buf.begin());
    ASSERT_EQ(0, *i);
    ++i;
    ASSERT_EQ(42, *i++);
    ASSERT_EQ(-666, *i++);
    ASSERT_EQ(0x12345689abcdefLL, *i++);
    ASSERT_EQ(0x77777777777777LL, *i++);
    ASSERT_EQ(-0x66666666666666LL, *i++);
    ASSERT_EQ(13, *i++);
    ASSERT_EQ(i, buf.end());
    ASSERT_EQ(13, *--i);
    --i;
    ASSERT_EQ(-0x66666666666666LL, *i--);
    ASSERT_EQ(0x77777777777777LL, *i);

    auto r = buf.rbegin();
    ASSERT_EQ(13, *r++);
    ASSERT_EQ(-0x66666666666666LL, *r++);
    ASSERT_EQ(0x77777777777777LL, *r++);
    ASSERT_EQ(0x12345689abcdefLL, *r++);
    ASSERT_EQ(-666, *r++);
    ASSERT_EQ(42, *r++);
    ASSERT_EQ(0, *r++);
    ASSERT_EQ(r, buf.rend());
    --r;
    --r;
    ASSERT_EQ(42, *r--);
    ASSERT_EQ(-666, *r--);
    ASSERT_EQ(0x12345689abcdefLL, *r--);
    ASSERT_EQ(0x77777777777777LL, *r--);
    ASSERT_EQ(-0x66666666666666LL, *r--);
    ASSERT_EQ(13, *r);
    ASSERT_EQ(r, buf.rbegin());
}

TEST(IR, Code)
{
    ir::code code;

    ASSERT_EQ(0, code(ir::Add(-42, 42)));
    ASSERT_EQ(5, code(ir::Invoke(1, 2, 3)));
    ASSERT_EQ(11, code(ir::Sub(666, 1234567890)));
/*
    auto writer = ir::debug_writer();
    code.pass(writer);
    std::cout << writer.str() << std::endl;
*/
    ir::word index = 0;
    auto first = code.read(index);
    auto second = code.read(index);
    auto third = code.read(index);

    ASSERT_EQ(index, code.size());

    ASSERT_EQ("Add", first->name());
    ASSERT_EQ("Invoke", second->name());
    ASSERT_EQ("Sub", third->name());

    ASSERT_EQ(2, first->nargs());
    ASSERT_EQ(3, second->nargs());
    ASSERT_EQ(2, third->nargs());

    ASSERT_EQ(-42, (*first)[0]);
    ASSERT_EQ(42, (*first)[1]);
    ASSERT_EQ(1, (*second)[0]);
    ASSERT_EQ(2, (*second)[1]);
    ASSERT_EQ(3, (*second)[2]);
    ASSERT_EQ(666, (*third)[0]);
    ASSERT_EQ(1234567890, (*third)[1]);
}
