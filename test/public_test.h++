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
