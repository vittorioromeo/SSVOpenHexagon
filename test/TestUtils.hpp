#pragma once

#include "SFML/System/IO.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtAppendMixin.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: keep -- fmtTo formats numeric args (line numbers)
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/CommonType.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


#define TEST_MAIN(...) int __attribute__((const)) main(__VA_ARGS__)

#define TEST_IMPL_DEPARENS(...) __VA_ARGS__

#define SA_SAME(value, type) static_assert(SFML_BASE_IS_SAME(TEST_IMPL_DEPARENS value, TEST_IMPL_DEPARENS type))

#define SA_TYPE(value, type) static_assert(SFML_BASE_IS_SAME(decltype value, TEST_IMPL_DEPARENS type))

namespace test_impl::impl
{

inline auto& get_ostringstream() noexcept
{
    static sf::base::String oss;
    return oss;
}

inline auto& clear_and_get_ostringstream() noexcept
{
    auto& oss(get_ostringstream());
    oss.clear();
    return oss;
}

[[noreturn]] inline void fail() noexcept
{
    sf::base::printLn("{}", get_ostringstream());
    sf::base::abort();
}

inline void output_header(sf::base::String& s)
{
    s += "____________________________________________________________________"
         "\nTEST FAILED:\n";
}

inline void output_line(sf::base::String& s, int line)
{
    s.appendFmt("*     line: `{}`\n", line);
}

inline void output_expr(sf::base::String& s, const char* expr)
{
    s.appendFmt("*     expr: `{}`\n", expr);
}

template <typename T>
void output_result(sf::base::String& s, const T& lhs_result)
{
    s.appendFmt("*   result: `{}`\n", lhs_result);
}

inline void output_expected(sf::base::String& s, const char* expected)
{
    s.appendFmt("* expected: `{}`\n", expected);
}

template <typename T>
void output_expected(sf::base::String& s, const char* expected, const T& rhs_result)
{
    s.appendFmt("* expected: `{}` (which evaluates to `{}`)\n", expected, rhs_result);
}

template <typename TF>
void do_test(bool x, TF&& f)
{
    if (x) [[likely]]
    {
        return;
    }

    auto& oss = impl::clear_and_get_ostringstream();
    f(oss);
    impl::fail();
}

} // namespace test_impl::impl

namespace test_impl
{

template <typename T>
inline auto test_expr(int line, bool x, T&& lhs_result, const char* expr) noexcept
{
    return impl::do_test(x,
                         [&](auto& s)
    {
        impl::output_header(s);
        impl::output_line(s, line);
        impl::output_expr(s, expr);
        impl::output_result(s, lhs_result);
    });
}

template <typename TLhs, typename TRhs>
inline auto test_op(int line, bool x, TLhs&& lhs_result, TRhs&& rhs_result, const char* expr, const char* expected)
{
    return impl::do_test(x,
                         [&](auto& s)
    {
        impl::output_header(s);
        impl::output_line(s, line);
        impl::output_expr(s, expr);
        impl::output_result(s, lhs_result);
        impl::output_expected(s, expected, rhs_result);
    });
}

inline auto test_expr_ns(int line, bool x, const char* expr) noexcept
{
    return impl::do_test(x,
                         [&](auto& s)
    {
        impl::output_header(s);
        impl::output_line(s, line);
        impl::output_expr(s, expr);
    });
}

inline auto test_op_ns(int line, bool x, const char* expr, const char* expected)
{
    return impl::do_test(x,
                         [&](auto& s)
    {
        impl::output_header(s);
        impl::output_line(s, line);
        impl::output_expr(s, expr);
        impl::output_expected(s, expected);
    });
}

} // namespace test_impl

#define TEST_ASSERT(expr)                                                     \
    do                                                                        \
    {                                                                         \
        auto _t_x(expr);                                                      \
                                                                              \
        test_impl::test_expr(__LINE__, static_cast<bool>(_t_x), expr, #expr); \
    } while (false)

#define TEST_ASSERT_OP(lhs, op, rhs)                                                                      \
    do                                                                                                    \
    {                                                                                                     \
        using ct = SFML_BASE_COMMON_TYPE(decltype(lhs), decltype(rhs));                                   \
                                                                                                          \
        auto _t_xl(lhs);                                                                                  \
        auto _t_xr(rhs);                                                                                  \
                                                                                                          \
        auto _t_x(ct(_t_xl) op ct(_t_xr));                                                                \
                                                                                                          \
        test_impl::test_op(__LINE__, static_cast<bool>(_t_x), _t_xl, _t_xr, #lhs " " #op " " #rhs, #rhs); \
    } while (false)

#define TEST_ASSERT_EQ(lhs, rhs) TEST_ASSERT_OP(lhs, ==, rhs)
#define TEST_ASSERT_NE(lhs, rhs) TEST_ASSERT_OP(lhs, !=, rhs)
#define TEST_ASSERT_LT(lhs, rhs) TEST_ASSERT_OP(lhs, <, rhs)
#define TEST_ASSERT_LE(lhs, rhs) TEST_ASSERT_OP(lhs, <=, rhs)
#define TEST_ASSERT_GT(lhs, rhs) TEST_ASSERT_OP(lhs, >, rhs)
#define TEST_ASSERT_GE(lhs, rhs) TEST_ASSERT_OP(lhs, >=, rhs)

// ----------------------------------------------------------------------------

#define TEST_ASSERT_NS(expr)                                               \
    do                                                                     \
    {                                                                      \
        auto _t_x(expr);                                                   \
                                                                           \
        test_impl::test_expr_ns(__LINE__, static_cast<bool>(_t_x), #expr); \
    } while (false)

#define TEST_ASSERT_NS_OP(lhs, op, rhs)                                                        \
    do                                                                                         \
    {                                                                                          \
        auto _t_xl(lhs);                                                                       \
        auto _t_x(_t_xl op rhs);                                                               \
                                                                                               \
        test_impl::test_op_ns(__LINE__, static_cast<bool>(_t_x), #lhs " " #op " " #rhs, #rhs); \
    } while (false)

#define TEST_ASSERT_NS_EQ(lhs, rhs) TEST_ASSERT_NS_OP(lhs, ==, rhs)
#define TEST_ASSERT_NS_NE(lhs, rhs) TEST_ASSERT_NS_OP(lhs, !=, rhs)
#define TEST_ASSERT_NS_LT(lhs, rhs) TEST_ASSERT_NS_OP(lhs, <, rhs)
#define TEST_ASSERT_NS_LE(lhs, rhs) TEST_ASSERT_NS_OP(lhs, <=, rhs)
#define TEST_ASSERT_NS_GT(lhs, rhs) TEST_ASSERT_NS_OP(lhs, >, rhs)
#define TEST_ASSERT_NS_GE(lhs, rhs) TEST_ASSERT_NS_OP(lhs, >=, rhs)
