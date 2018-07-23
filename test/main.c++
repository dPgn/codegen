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

#include "x86_asm_test.h++"

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
