// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <utility>

namespace hg::Utils
{

template <typename F>
struct scope_guard : F
{
    explicit scope_guard(F&& f) noexcept : F{std::move(f)}
    {
    }

    ~scope_guard() noexcept
    {
        static_cast<F&> (*this)();
    }
};

} // namespace hg::Utils
