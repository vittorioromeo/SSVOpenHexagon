// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SFML/System/IO.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringStreamOp.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsConvertible.hpp"
#include "SFML/Base/Trait/IsSame.hpp"

#include <string_view>

namespace hg::Utils
{

namespace Impl
{

template <typename T>
struct IsCharArray : std::false_type
{
};

template <sf::base::SizeT N>
struct IsCharArray<char[N]> : std::true_type
{
};

template <sf::base::SizeT N>
struct IsCharArray<const char[N]> : std::true_type
{
};

template <sf::base::SizeT N>
struct IsCharArray<char (&)[N]> : std::true_type
{
};

template <sf::base::SizeT N>
struct IsCharArray<const char (&)[N]> : std::true_type
{
};

template <typename... Ts>
inline constexpr bool AllConvertibleToStringView = ((SFML_BASE_IS_CONVERTIBLE(Ts, std::string_view) ||
                                                     SFML_BASE_IS_CONVERTIBLE(Ts, sf::base::StringView) ||
                                                     IsCharArray<Ts>::value || SFML_BASE_IS_SAME(Ts, char)) &&
                                                    ...);

template <sf::base::SizeT N>
[[nodiscard, gnu::always_inline]] inline constexpr sf::base::SizeT getSize(char (&)[N]) noexcept
{
    return N;
}

template <sf::base::SizeT N>
[[nodiscard, gnu::always_inline]] inline constexpr sf::base::SizeT getSize(const char (&)[N]) noexcept
{
    return N;
}

[[nodiscard, gnu::always_inline]] inline constexpr sf::base::SizeT getSize(const char* s) noexcept
{
    const char* end = s;
    while (*end++ != 0)
    {
    }
    return end - s - 1;
}

[[nodiscard, gnu::always_inline]] inline constexpr sf::base::SizeT getSize(const char&) noexcept
{
    return 1;
}

[[nodiscard, gnu::always_inline]] inline sf::base::SizeT getSize(const sf::base::String& s) noexcept
{
    return s.size();
}

[[nodiscard, gnu::always_inline]] inline constexpr sf::base::SizeT getSize(const std::string_view& s) noexcept
{
    return s.size();
}

} // namespace Impl

template <typename... Ts>
[[nodiscard]] sf::base::String concat(const Ts&... xs)
    requires(!Impl::AllConvertibleToStringView<Ts...>)
{
    thread_local sf::OutStringStream oss;
    oss.setStr("");

    (oss << ... << xs);
    return oss.to<sf::base::String>();
}

template <typename... Ts>
void concatInto(sf::base::String& result, const Ts&... xs)
    requires(Impl::AllConvertibleToStringView<Ts...>)
{
    const sf::base::SizeT space = (1 + ... + Impl::getSize(xs));
    result.reserve(result.size() + space);
    ((result += xs), ...);
}

template <typename... Ts>
[[nodiscard]] sf::base::String concat(const Ts&... xs)
    requires(Impl::AllConvertibleToStringView<Ts...>)
{
    sf::base::String result;
    concatInto(result, xs...);
    return result;
}

} // namespace hg::Utils
