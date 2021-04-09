// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <cstddef>

namespace hg {

template <typename T>
[[gnu::always_inline, nodiscard]] inline constexpr std::size_t toSizeT(
    const T value) noexcept
{
    return static_cast<std::size_t>(value);
}

} // namespace hg
