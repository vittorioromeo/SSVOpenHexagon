// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

#include <type_traits>
#include <variant>
#include <utility>

namespace hg::Utils
{

template <typename... Fs>
struct overload_set : Fs...
{
    template <typename... FFwds>
    constexpr overload_set(FFwds&&... fFwds) : Fs{std::forward<FFwds>(fFwds)}...
    {
    }

    using Fs::operator()...;
};

template <typename... Fs>
overload_set(Fs...) -> overload_set<Fs...>;

template <typename... Fs>
constexpr auto make_overload_set(Fs&&... fs)
{
    return overload_set<std::decay_t<Fs>...>{std::forward<Fs>(fs)...};
}

template <typename Variant, typename... Fs>
constexpr decltype(auto) match(Variant&& v, Fs&&... fs)
{
    return std::visit(
        make_overload_set(std::forward<Fs>(fs)...), std::forward<Variant>(v));
}

} // namespace hg::Utils
