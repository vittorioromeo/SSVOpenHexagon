// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <cstddef>

namespace hg::Utils {

namespace Impl {

template <typename T>
struct IsCharArray : std::false_type
{};

template <std::size_t N>
struct IsCharArray<char[N]> : std::true_type
{};

template <std::size_t N>
struct IsCharArray<const char[N]> : std::true_type
{};

template <std::size_t N>
struct IsCharArray<char (&)[N]> : std::true_type
{};

template <std::size_t N>
struct IsCharArray<const char (&)[N]> : std::true_type
{};

template <typename... Ts>
inline constexpr bool AllConvertibleToStringView =
    ((std::is_convertible_v<Ts, std::string_view> || IsCharArray<Ts>::value ||
        std::is_same_v<Ts, char>)&&...);

template <std::size_t N>
[[nodiscard, gnu::always_inline]] constexpr inline std::size_t getSize(
    char (&)[N]) noexcept
{
    return N;
}

template <std::size_t N>
[[nodiscard, gnu::always_inline]] constexpr inline std::size_t getSize(
    const char (&)[N]) noexcept
{
    return N;
}

[[nodiscard, gnu::always_inline]] constexpr inline std::size_t getSize(
    const char* s) noexcept
{
    const char* end = s;
    while(*end++ != 0)
    {
    }
    return end - s - 1;
}

[[nodiscard, gnu::always_inline]] constexpr inline std::size_t getSize(
    const char&) noexcept
{
    return 1;
}

[[nodiscard, gnu::always_inline]] inline std::size_t getSize(
    const std::string& s) noexcept
{
    return s.size();
}

[[nodiscard, gnu::always_inline]] constexpr inline std::size_t getSize(
    const std::string_view& s) noexcept
{
    return s.size();
}

} // namespace Impl

template <typename... Ts>
[[nodiscard]] auto concat(const Ts&... xs)
    -> std::enable_if_t<!Impl::AllConvertibleToStringView<Ts...>, std::string>
{
    thread_local std::ostringstream oss;
    oss.str("");

    (oss << ... << xs);
    return oss.str();
}

template <typename... Ts>
auto concatInto(std::string& result, const Ts&... xs)
    -> std::enable_if_t<Impl::AllConvertibleToStringView<Ts...>, void>
{
    const std::size_t space = (1 + ... + Impl::getSize(xs));
    result.reserve(result.size() + space);
    ((result += xs), ...);
}

template <typename... Ts>
[[nodiscard]] auto concat(const Ts&... xs)
    -> std::enable_if_t<Impl::AllConvertibleToStringView<Ts...>, std::string>
{
    std::string result;
    concatInto(result, xs...);
    return result;
}

} // namespace hg::Utils
