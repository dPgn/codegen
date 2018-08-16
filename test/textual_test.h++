#include "../textual.h++"

TEST(Textual, Minimal)
{
    textual txt("[i64: Int -64][fun: Enter 0 [Fun i64 i64 i64]][Exit fun]");
    std::string s = txt.code().text();
    textual txt2(s);
    // Not yet implemented!!
//    ASSERT_EQ(s, txt2.code().text());
}
