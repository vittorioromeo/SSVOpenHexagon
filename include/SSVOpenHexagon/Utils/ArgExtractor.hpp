// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <cstddef>
#include <tuple>

namespace hg::Utils {

template <typename>
struct ArgExtractor;

template <typename R, typename F, typename... Args>
struct ArgExtractor<R (F::*)(Args...)>
{
    using Return = R;
    using Function = F;

    enum
    {
        numArgs = sizeof...(Args)
    };

    template <std::size_t I>
    using NthArg = std::tuple_element_t<I, std::tuple<Args...>>;
};

template <typename R, typename F, typename... Args>
struct ArgExtractor<R (F::*)(Args...) const> : ArgExtractor<R (F::*)(Args...)>
{
    using ArgExtractor<R (F::*)(Args...)>::NthArg;
};

} // namespace hg::Utils
