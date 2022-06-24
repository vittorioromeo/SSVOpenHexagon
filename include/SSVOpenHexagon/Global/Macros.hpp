// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

namespace hg::Impl {

template <typename T>
struct RemoveRef
{
    using type = T;
};

template <typename T>
struct RemoveRef<T&>
{
    using type = T;
};

template <typename T>
struct RemoveRef<T&&>
{
    using type = T;
};

} // namespace hg::Impl

#define SSVOH_MOVE(...)                                                 \
    static_cast<                                                        \
        typename ::hg::Impl::RemoveRef<decltype(__VA_ARGS__)>::type&&>( \
        __VA_ARGS__)

#define SSVOH_FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
