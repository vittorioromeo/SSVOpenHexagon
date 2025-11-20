// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <string>
#include <sstream>
#include <string_view>

#include <SFML/Base/SizeT.hpp>
#include <SFML/Base/Trait/IsConvertible.hpp>
#include <SFML/Base/Trait/IsSame.hpp>

namespace hg::Utils {

namespace Impl {

template <typename T>
struct IsCharArray : std::false_type
{};

template <sf::base::SizeT N>
struct IsCharArray<char[N]> : std::true_type
{};

template <sf::base::SizeT N>
struct IsCharArray<const char[N]> : std::true_type
{};

template <sf::base::SizeT N>
struct IsCharArray<char (&)[N]> : std::true_type
{};

template <sf::base::SizeT N>
struct IsCharArray<const char (&)[N]> : std::true_type
{};

template <typename... Ts>
inline constexpr bool AllConvertibleToStringView =
    ((SFML_BASE_IS_CONVERTIBLE(Ts, std::string_view) ||
         IsCharArray<Ts>::value || SFML_BASE_IS_SAME(Ts, char)) &&
        ...);

template <sf::base::SizeT N>
[[nodiscard, gnu::always_inline]] constexpr inline sf::base::SizeT getSize(
    char (&)[N]) noexcept
{
    return N;
}

template <sf::base::SizeT N>
[[nodiscard, gnu::always_inline]] constexpr inline sf::base::SizeT getSize(
    const char (&)[N]) noexcept
{
    return N;
}

[[nodiscard, gnu::always_inline]] constexpr inline sf::base::SizeT getSize(
    const char* s) noexcept
{
    const char* end = s;
    while (*end++ != 0)
    {
    }
    return end - s - 1;
}

[[nodiscard, gnu::always_inline]] constexpr inline sf::base::SizeT getSize(
    const char&) noexcept
{
    return 1;
}

[[nodiscard, gnu::always_inline]] inline sf::base::SizeT getSize(
    const std::string& s) noexcept
{
    return s.size();
}

[[nodiscard, gnu::always_inline]] constexpr inline sf::base::SizeT getSize(
    const std::string_view& s) noexcept
{
    return s.size();
}

} // namespace Impl

template <typename... Ts>
[[nodiscard]] std::string concat(const Ts&... xs)
    requires(!Impl::AllConvertibleToStringView<Ts...>)
{
    thread_local std::ostringstream oss;
    oss.str("");

    (oss << ... << xs);
    return oss.str();
}

template <typename... Ts>
void concatInto(std::string& result, const Ts&... xs)
    requires(Impl::AllConvertibleToStringView<Ts...>)
{
    const sf::base::SizeT space = (1 + ... + Impl::getSize(xs));
    result.reserve(result.size() + space);
    ((result += xs), ...);
}

template <typename... Ts>
[[nodiscard]] std::string concat(const Ts&... xs)
    requires(Impl::AllConvertibleToStringView<Ts...>)
{
    std::string result;
    concatInto(result, xs...);
    return result;
}

} // namespace hg::Utils
